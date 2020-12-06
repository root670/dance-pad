"""Configurable settings.
"""
import os
from dotenv import load_dotenv


load_dotenv()

CONFIG = dict(
    SERIAL_DEVICE=os.getenv('SERIAL_DEVICE')
)
