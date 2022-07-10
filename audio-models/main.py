import logging
import os
from argparse import ArgumentParser
from typing import Dict, List
import librosa
import soundfile as sf

from scipy.io.wavfile import write, read
import torch as tr
import torch.nn as nn
from torch import Tensor

from typing import Tuple

logging.basicConfig()
log = logging.getLogger(__name__)
log.setLevel(level=os.environ.get("LOGLEVEL", "INFO"))

from models import models

import numpy as np
import matplotlib.pyplot as plt

SHAPE: Tuple = (1, 2, 44100)

x = np.linspace(0, 1, 44100)
y = np.sin(2 * np.pi * 220 * x)
sin_tensor = tr.tensor(y)
sin_tensor = sin_tensor.reshape((1, 44100))
y_stereo = tr.tensor([y, y]).reshape(SHAPE)

# padding_cached = models.PaddingCached(SHAPE)


def run_model(model=None, y_stereo=None, filename_path="None", **kwargs):
    global processed_signal, processed_signal_numpy
    global left_channel, right_channel, audio
    processed_signal = model.forward(y_stereo, **kwargs)
    processed_signal_numpy = processed_signal.numpy().reshape((2, 44100))
    audio = []
    for i in range(44100):
        sample = []
        sample.append(processed_signal_numpy[0, i])
        sample.append(processed_signal_numpy[1, i])

        audio.append(sample)

    sf.write(filename_path, audio, 44100)
    print(f"saved {filename_path}")

    # write(filename_path, 44100, audio)
    # write(filename_path, 44100, processed_signal_numpy)
    # print(filename_path)


def export_model(model, modelname_path):
    sm = tr.jit.script(model)
    sm.save(modelname_path)
    print(f"model saved {modelname_path}")


# def main():
if __name__ == "__main__":
    clean_signal = tr.tensor([y, y]).reshape(SHAPE)
    dist_model = models.ClipperModelFixed()
    run_model(dist_model, clean_signal, "audio/sinusoid_distorted.wav")
    export_model(dist_model, "torch-models/ClipperTSFinal.pt")

    clean_signal = tr.tensor([y, y]).reshape(SHAPE)
    channel_killer = models.ChannelKiller()
    run_model(channel_killer, clean_signal, "audio/channel_killer.wav")
    export_model(channel_killer, "torch-models/ChannelKillerTS.pt")

    params_fx = {"fs": 44100, "block_size": 44100, "n_channels": 2}
    clean_signal = tr.tensor([y, y]).reshape(SHAPE)
    fx_model = models.Effects()
    run_model(fx_model, clean_signal, "audio/torch_fx.wav", **params_fx)
    export_model(fx_model, "torch-models/TorchFxTS.pt")

    clean_signal = tr.tensor([y, y]).reshape(SHAPE)
    dst_atan = models.ArcTanDistortion()
    run_model(dst_atan, clean_signal, "audio/atan_dst.wav")
    export_model(dst_atan, "torch-models/ATanDistortionTS.pt")
