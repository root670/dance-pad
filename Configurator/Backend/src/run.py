from communicator import Communicator
from time import perf_counter

c = Communicator()

t = perf_counter()
c.get_version()
print(perf_counter() - t)

t = perf_counter()
c.get_panel_config()
print(perf_counter() - t)

t = perf_counter()
c.get_sensor_values()
print(perf_counter() - t)

t = perf_counter()
c.blink_led()
print(perf_counter() - t)