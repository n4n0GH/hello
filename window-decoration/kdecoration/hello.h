#ifndef hello_h
#define hello_h

/*
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hellosettings.h"

#include <QSharedPointer>
#include <QList>

namespace Hello
{
    //* convenience typedefs
    using InternalSettingsPtr = QSharedPointer<InternalSettings>;
    using InternalSettingsList = QList<InternalSettingsPtr>;
    using InternalSettingsListIterator = QListIterator<InternalSettingsPtr>;

    //* metrics
    enum Metrics
    {

        //* corner radius (pixels)
        Frame_FrameRadiusNone = 0,
        Frame_FrameRadiusTiny = 3,
        Frame_FrameRadius = 5,
        Frame_FrameRadiusExtended = 7,

        //* titlebar metrics, in units of small spacing
        TitleBar_MarginSmall = 1,
        TitleBar_Margin = 3,
        TitleBar_MarginLarge = 5,
        TitleBar_TopMargin = 3,
        TitleBar_BottomMargin = 3,
        TitleBar_SideMarginSmall = 2,
        TitleBar_SideMargin = 4,
        TitleBar_SideMarginLarge = 6,
        TitleBar_ButtonSpacingSmall = 2,
        TitleBar_ButtonSpacing = 4,
        TitleBar_ButtonSpacingLarge = 6,

        // shadow dimensions (pixels)
        Shadow_Overlap = 3,

    };

    //* exception
    enum ExceptionMask
    {
        None = 0,
        BorderSize = 1<<4
    };
}

#endif
