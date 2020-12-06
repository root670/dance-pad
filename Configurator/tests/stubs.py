"""Stub values for tests
"""
from random import sample


PANEL_CONFIG_RESPONSE = b'0,270,180,90\n'
SENSOR_VALUES_RESPONSE = '{values}\n'.format(
    values=','.join(map(str, sample(range(1024), 16)))
).encode('ascii')
