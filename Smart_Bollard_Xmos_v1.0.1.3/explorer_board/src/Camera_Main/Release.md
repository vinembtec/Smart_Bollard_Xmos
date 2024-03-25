# 1.0.1.3
1. Camera shutdown between images is removed
2. No exit loop variable is hardcoded
3. Mipi tasks runs at higher priority
4. Stack size of Rtos tasks doubled
5. capture task priority is lowered\
6. Continuous 3 Seconds timer in MipiImager task
7. Debug code put for monitoring MIPI headers via GPIO in mipi imager task 