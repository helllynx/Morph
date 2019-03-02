import abc
import typing


class Store:
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def dispatch(self, action: typing.Dict) -> None:
        pass

    @abc.abstractmethod
    def subscribe(self, callback: typing.Callable) -> None:
        pass

    @property
    @abc.abstractmethod
    def state(self):
        pass
