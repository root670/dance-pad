"""Stub values for tests
"""
from random import sample


GET_CONFIG_RESPONSE = b'0,1,2,3,4,270,5,6,7,8,180,9,10,11,12,90,13,14,15,16\n'
SENSOR_VALUES_RESPONSE = '{values}\n'.format(
    values=','.join(map(str, sample(range(1024), 16)))
).encode('ascii')

SENSOR_VALUES_RESPONSE = '{values}\n'.format(
    values=','.join([a for t in [*zip(
        map(str, sample(range(1024), 16)), # pressure
        map(str, sample(range(500, 600), 16)), # trigger threshold
        map(str, sample(range(300, 400), 16)), # release threshold
    )] for a in t])
).encode('ascii')