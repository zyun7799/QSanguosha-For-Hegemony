/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "audio.h"
#include "settings.h"

#include <QCache>
#include <fmod.hpp>

using namespace FMOD;

class QSanSound;

static System *fmSystem;
static QCache<QString, QSanSound> SoundCache;
static Sound *BGM;
static Channel *BGMChannel;

class QSanSound {
public:
    QSanSound(const QString &filename) : sound(NULL), channel(NULL) {
        fmSystem->createSound(filename.toLatin1(), FMOD_DEFAULT, NULL, &sound);
    }

    ~QSanSound() {
        if (sound) {
            if (channel)
                channel->stop();

            sound->release();
        }
    }

    void play(const bool doubleVolume = false) {
        if (sound) {
            FMOD_RESULT result = fmSystem->playSound(sound, NULL, false, &channel);

            if (result == FMOD_OK) {
                channel->setVolume((doubleVolume ? 2 : 1) * Config.EffectVolume);
                fmSystem->update();
            }
        }
    }

    bool isPlaying() const {
        if (channel == NULL) return false;

        bool p = false;
        channel->isPlaying(&p);

        return p;
    }

    void stop() {
        if (channel)
            channel->stop();
    }

private:
    Sound *sound;
    Channel *channel;
};

void QSanAudio::init() {
    FMOD_RESULT result = System_Create(&fmSystem);
    if (result == FMOD_OK)
        fmSystem->init(100, 0, NULL);
}

void QSanAudio::quit() {
    if (fmSystem) {
        SoundCache.clear();
        fmSystem->release();

        fmSystem = NULL;
    }
}

void QSanAudio::play(const QString &filename) {
    QSanSound *sound = SoundCache[filename];
    if (sound == NULL) {
        sound = new QSanSound(filename);
        SoundCache.insert(filename, sound);
    } else if (sound->isPlaying()) {
        return;
    }

    sound->play();
}

void QSanAudio::playAudioOfMoxuan()
{
    QSanSound *sound = new QSanSound("audio/system/moxuan.ogg");
    SoundCache.insert("audio/system/moxuan.ogg", sound);
    sound->play(true);
}

void QSanAudio::stop() {
    if (system == NULL) return;

    QStringList cacheKeys = SoundCache.keys();

    foreach (const QString &key, cacheKeys) {
        QSanSound *sound = SoundCache[key];
        if (sound != NULL && sound->isPlaying())
            sound->stop();
    }

    stopBGM();

    fmSystem->update();
}

void QSanAudio::playBGM(const QString &filename) {
    FMOD_RESULT result = fmSystem->createStream(filename.toLatin1(), FMOD_LOOP_NORMAL, NULL, &BGM);

    if (result == FMOD_OK) {
        BGM->setLoopCount(-1);
        fmSystem->playSound(BGM, NULL, false, &BGMChannel);

        BGMChannel->setVolume(Config.BGMVolume);
        fmSystem->update();
    }
}

void QSanAudio::setBGMVolume(float volume) {
    if (BGMChannel)
        BGMChannel->setVolume(volume);
}

void QSanAudio::stopBGM() {
    if (BGMChannel)
        BGMChannel->stop();
}

QString QSanAudio::getVersion() {
    unsigned int version = 0;
    fmSystem->getVersion(&version);
    // convert it to QString
    return QString("%1.%2.%3").arg((version & 0xFFFF0000) >> 16, 0, 16)
        .arg((version & 0xFF00) >> 8, 2, 16, QChar('0'))
        .arg((version & 0xFF), 2, 16, QChar('0'));
}

