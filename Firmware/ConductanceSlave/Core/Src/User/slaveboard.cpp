// ============================================================================================== //
//                                                                                                //
//  This file is part of the ISF ReDeX project.                                                   //
//                                                                                                //
//  Author:                                                                                       //
//  Marcel Hasler <mahasler@gmail.com>                                                            //
//                                                                                                //
//  Copyright (c) 2021 - 2023                                                                     //
//  Bonn-Rhein-Sieg University of Applied Sciences                                                //
//                                                                                                //
//  This program is free software: you can redistribute it and/or modify it under the terms       //
//  of the GNU General Public License as published by the Free Software Foundation, either        //
//  version 3 of the License, or (at your option) any later version.                              //
//                                                                                                //
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;     //
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     //
//  See the GNU General Public License for more details.                                          //
//                                                                                                //
//  You should have received a copy of the GNU General Public License along with this program.    //
//  If not, see <https://www.gnu.org/licenses/>.                                                  //
//                                                                                                //
// ============================================================================================== //

#include "assert.h"
#include "main.h"
#include "slaveboard.h"

// ---------------------------------------------------------------------------------------------- //

SlaveBoard* SlaveBoard::s_instance = nullptr;

// ---------------------------------------------------------------------------------------------- //

void HAL_GPIO_EXTI_Falling_Callback(uint16_t pin)
{
    ASSERT(pin == COMM_CS_Pin);

    if (SlaveBoard::s_instance != nullptr)
        SlaveBoard::s_instance->handleExternalInterruptFalling();
}

// ---------------------------------------------------------------------------------------------- //

void HAL_GPIO_EXTI_Rising_Callback(uint16_t pin)
{
    ASSERT(pin == COMM_CS_Pin);

    if (SlaveBoard::s_instance != nullptr)
        SlaveBoard::s_instance->handleExternalInterruptRising();
}

// ---------------------------------------------------------------------------------------------- //

SlaveBoard::SlaveBoard()
    : m_currentCommand(0),
      m_unhandledCommand(0),
      m_commandParser(this)
{
    s_instance = this;
}

// ---------------------------------------------------------------------------------------------- //

SlaveBoard::~SlaveBoard()
{
    HAL_GPIO_WritePin(COMM_CON_GPIO_Port, COMM_CON_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_SET);

    HAL_OPAMP_Stop(Config::VoutOpAmpHandle);

    s_instance = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void SlaveBoard::exec()
{
    HAL_OPAMP_Start(Config::VoutOpAmpHandle);

    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COMM_CON_GPIO_Port, COMM_CON_Pin, GPIO_PIN_RESET);

    while (true)
    {
        if (m_unhandledCommand)
        {
            const uint32_t command = m_unhandledCommand;
            m_unhandledCommand = 0;
            m_commandParser.parse(command);
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

void SlaveBoard::handleExternalInterruptFalling()
{
    static constexpr size_t SampleCount = 2;

    static const auto txFifoSynchronized = [](SPI_HandleTypeDef* spi) {
        return (spi->Instance->SR & SPI_SR_FTLVL_Msk) >> SPI_SR_FTLVL_Pos != 0x02;
    };

    // Cancel previous transmission if necessary
    if (HAL_DMA_GetState(m_commDmaRxHandle) != HAL_DMA_STATE_READY)
        HAL_DMA_Abort(m_commDmaRxHandle);

    // Internal slave select
    m_commSpiHandle->Instance->CR1 &= ~SPI_CR1_SSI;

    // Clear spurious data
    while (__HAL_SPI_GET_FLAG(m_commSpiHandle, SPI_FLAG_RXNE) == SET)
        m_commSpiHandle->Instance->DR;

    // Enable receive DMA stream & request
    HAL_DMA_Start(m_commDmaRxHandle,
                  reinterpret_cast<uint32_t>(&m_commSpiHandle->Instance->DR),
                  reinterpret_cast<uint32_t>(&m_currentCommand),
                  SampleCount);

    SET_BIT(m_commSpiHandle->Instance->CR2, SPI_CR2_RXDMAEN);

    // Enable SPI
    __HAL_SPI_ENABLE(m_commSpiHandle);

    // Only write first sample to 32-bit FIFO if we are synchronized.
    // This is necessary in order to work around an apparent µC bug.
    if (txFifoSynchronized(m_commSpiHandle))
        m_commSpiHandle->Instance->DR = m_signalReader.voltageValue();

    m_commSpiHandle->Instance->DR = m_signalReader.currentValue();

    // Signal master
    HAL_GPIO_WritePin(COMM_DRDY_GPIO_Port, COMM_DRDY_Pin, GPIO_PIN_RESET);

    // Update output signal
    m_signalGenerator.update();
}

// ---------------------------------------------------------------------------------------------- //

void SlaveBoard::handleExternalInterruptRising()
{
    HAL_DMA_PollForTransfer(m_commDmaRxHandle, HAL_DMA_FULL_TRANSFER, 0);
    HAL_GPIO_WritePin(COMM_DRDY_GPIO_Port, COMM_DRDY_Pin, GPIO_PIN_SET);

    // Internal slave release
    m_commSpiHandle->Instance->CR1 |= SPI_CR1_SSI;

    // Disable DMA request
    CLEAR_BIT(m_commSpiHandle->Instance->CR2, SPI_CR2_RXDMAEN);

    // Disable SPI
    __HAL_SPI_DISABLE(m_commSpiHandle);

    // Start next conversion
    m_signalReader.update();

    // Save command if not busy
    if (m_currentCommand != 0)
        m_unhandledCommand = m_currentCommand;
}

// ---------------------------------------------------------------------------------------------- //

void SlaveBoard::onCommandSetupSignal(Waveform waveform, unsigned int frequency, double amplitude)
{
    m_signalGenerator.setup(waveform, frequency, amplitude);
}

// ---------------------------------------------------------------------------------------------- //

void SlaveBoard::onCommandSetGain(Gain gain)
{
    m_gainMux.setGain(gain);
}

// ---------------------------------------------------------------------------------------------- //

void SlaveBoard::onCommandReset()
{
    m_gainMux.setGain(Gain::_100);
    m_signalGenerator.reset();
}

// ---------------------------------------------------------------------------------------------- //
