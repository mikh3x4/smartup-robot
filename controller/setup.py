from setuptools import setup, find_packages

setup(
    name='smartup-robot',
    version='0.1.0',
    description='Setting up a python package',
    author='Michal',
    author_email='mikadam@stanford.edu',
    packages=find_packages(include=['robot.py', 'gamepad.py']),
    install_requires=[
        'hidapi',
    ],
)
