#pragma once

extern const unsigned long TIMECLOCK;

/**
 * @brief Get clock cycles
 *
 * @return mtime
 */
unsigned long get_cycles();

/**
 * @brief Set next supervisor timer interrupt
 *
 */
void clock_set_next_event();
