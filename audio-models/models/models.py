import torchaudio
import torch as tr
import torch.nn as nn
from torch import Tensor
import torch.nn.functional as F
import numpy as np
import math


class MultiChannelClipperModel(nn.Module):
    def __init__(self, min_val: float, max_val: float, gain_val: float) -> None:
        super().__init__()
        self.min_val = min_val
        self.max_val = max_val
        self.gain_val = gain_val

    def forward(self, x: Tensor) -> Tensor:
        for i in range(x.shape[-1]):
            for channel in range(x.shape[1]):
                for c in range(x.shape[0]):
                    x[c][channel][i] = tr.min(
                        tr.max(
                            x[c][channel][i],
                            tr.tensor(self.gain_val) * tr.tensor(-self.min_val),
                        ),
                        tr.tensor(self.gain_val) * tr.tensor(self.max_val),
                    )
                    # print(x[c][channel][i])
        return x


class ChannelKillerOriginal(nn.Module):
    def forward(self, x: Tensor) -> Tensor:
        for i in range(x.shape[-1]):
            print(f"i:{i}")
            for channel in range(x.shape[1]):
                for c in range(x.shape[0]):
                    if channel == 0:
                        x[c][channel][i] = x[c][channel][i] * 1
                    else:
                        x[c][channel][i] = x[c][channel][i] * 0.5
        return x


class ChannelKiller(nn.Module):
    def forward(self, x: Tensor) -> Tensor:
        for c in range(x.shape[0]):
            # print(f"c: {c}")
            for channel in range(x.shape[1]):
                # print(f"i:{channel}")
                for sample in range(x.shape[-1]):
                    if channel == 0:
                        x[c][channel][sample] = x[c][channel][sample] * 1
                    else:
                        x[c][channel][sample] = x[c][channel][sample] * 0
        return x


class ClipperModel(nn.Module):
    def __init__(self, min_val: float, max_val: float, gain_val: float) -> None:
        super().__init__()
        self.min_val = min_val
        self.max_val = max_val
        self.gain_val = gain_val

    def forward(self, x: Tensor) -> Tensor:
        for c in range(x.shape[0]):
            # print(f"c: {c}")
            for channel in range(x.shape[1]):
                # print(f"i:{channel}")
                for sample in range(x.shape[-1]):
                    x[c][channel][sample] = tr.min(
                        tr.max(
                            x[c][channel][sample],
                            tr.tensor(self.gain_val) * tr.tensor(-self.min_val),
                        ),
                        tr.tensor(self.gain_val) * tr.tensor(self.max_val),
                    )
                    # print(x[c][channel][i])
        return x


class Net(nn.Module):
    def __init__(self, num_channels):
        super(Net, self).__init__()

        self.num_channels = num_channels

        self.conv1 = nn.Conv2d(3, self.num_channels, 3, stride=1, padding=1)
        self.conv2 = nn.Conv2d(
            self.num_channels, self.num_channels * 2, 3, stride=1, padding=1
        )
        self.conv3 = nn.Conv2d(
            self.num_channels * 2, self.num_channels * 4, 3, stride=1, padding=1
        )

        self.fc1 = nn.Linear(self.num_channels * 4 * 8 * 8, self.num_channels * 4)
        self.fc2 = nn.Linear(self.num_channels * 4, 6)

    def forward(self, x):
        # Empieza 3x64x64
        x = self.conv1(x)  # num_channels x 64 x 64
        x = F.relu(F.max_pool2d(x, 2))  # num_channels x 32 x 32
        x = self.conv2(x)  # num_channels*2 x 32 x32
        x = F.relu(F.max_pool2d(x, 2))  # num_channels*2 x 16 x 16
        x = self.conv3(x)  # num_channels*4 x16x16
        x = F.relu(F.max_pool2d(x, 2))  # num_channels*4 x 8 x 8

        # flatten
        x = x.view(-1, self.num_channels * 4 * 8 * 8)

        # fc
        x = self.fc1(x)
        x = F.relu(x)
        x = self.fc2(x)

        # log_softmax

        x = F.log_softmax(x, dim=1)

        return x


class Effects(nn.Module):
    @staticmethod
    def _apply_effects(x: Tensor, fs: int, block_size: int, n_channels: int):
        effects = [
            ["lowpass", "-1", "300"],
            ["rate", f"{fs}"],
            ["reverb", "-w"],
        ]

        x, _ = torchaudio.sox_effects.apply_effects_tensor(
            x.reshape(n_channels, block_size).float(), fs, effects
        )

        return x

    def forward(self, x: Tensor, fs: int, block_size: int, n_channels: int) -> Tensor:
        x = self._apply_effects(x, fs, block_size, n_channels)
        return x


class ArcTanDistortion(nn.Module):
    def forward(self, x: Tensor) -> Tensor:
        gain = 67
        for c in range(x.shape[0]):
            # print(f"c: {c}")
            for channel in range(x.shape[1]):
                # print(f"i:{channel}")
                for sample in range(x.shape[-1]):

                    out = (2.0 / math.pi) * math.atan(gain * x[c][channel][sample])
                    out = out / math.log(gain)
                    # print(x[c][channel][sample], "--->", out)
                    x[c][channel][sample] = tr.tensor(out)

        return x


class ClipperModelFixed(nn.Module):
    def __init__(
        self, min_val: float = 0.5, max_val: float = 1.0, gain_val: float = 1.0
    ) -> None:
        super().__init__()
        self.min_val = min_val
        self.max_val = max_val
        self.gain_val = gain_val

    def forward(self, x: Tensor) -> Tensor:
        for c in range(x.shape[0]):
            # print(f"c: {c}")
            for channel in range(x.shape[1]):
                # print(f"i:{channel}")
                for sample in range(x.shape[-1]):
                    x[c][channel][sample] = tr.min(
                        tr.max(
                            x[c][channel][sample],
                            tr.tensor(self.gain_val) * tr.tensor(-self.min_val),
                        ),
                        tr.tensor(self.gain_val) * tr.tensor(self.max_val),
                    )
                    # print(x[c][channel][i])
        return x
