import abc
import typing


class Knob:
    _metadata: typing.Dict

    def __init__(self, metadata: typing.Dict):
        self._metadata = metadata

    @property
    def metadata(self):
        return self._metadata

    @property
    def is_reference(self):
        pass

    @property
    @abc.abstractmethod
    def data(self):
        pass

    @property
    @abc.abstractmethod
    def type(self) -> str:
        pass


class Node:
    _metadata: typing.Dict
    _input_knobs: typing.Dict
    _output_knobs: typing.Dict

    def __init__(self, metadata: typing.Dict):
        self._metadata = metadata
        self._input_knobs = {}
        self._output_knobs = {}

    @property
    def metadata(self):
        return self._metadata

    def update_metadata(self, metadata: typing.Dict) -> None:
        self._metadata.update(metadata)

    def add_input_knob(self, knob_name, knob: Knob):
        self._input_knobs[knob_name] = knob

    def add_output_knob(self, knob_name, knob: Knob):
        self._output_knobs[knob_name] = knob

    @property
    def input_knobs(self):
        return self._input_knobs

    @property
    def output_knobs(self):
        return self._output_knobs

    @abc.abstractmethod
    def compute(self):
        pass

