/*
* Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
* Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
* Copyright 2018  Vlad Zagorodniy <vladzzag@gmail.com>
* Copyright 2019  Richard Kung <ranmak@gmail.com>
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

#include "breezewaydecoration.h"

#include "breezeway.h"
#include "breezewaysettingsprovider.h"
#include "config-breezeway.h"
#include "config/breezewayconfigwidget.h"

#include "breezewaybutton.h"
#include "breezewaysizegrip.h"

#include "breezewayboxshadowrenderer.h"

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>

#include <KConfigGroup>
#include <KColorUtils>
#include <KSharedConfig>
#include <KPluginFactory>

#include <QPainter>
#include <QTextStream>
#include <QTimer>

#if BREEZEWAY_HAVE_X11
#include <QX11Info>
#endif

#include <cmath>

K_PLUGIN_FACTORY_WITH_JSON(
    BreezewayDecoFactory,
    "breezeway.json",
    registerPlugin<Breezeway::Decoration>();
    registerPlugin<Breezeway::Button>(QStringLiteral("button"));
    registerPlugin<Breezeway::ConfigWidget>(QStringLiteral("kcmodule"));
)

namespace
{
    struct ShadowParams {
        ShadowParams()
            : offset(QPoint(0, 0))
            , radius(0)
            , opacity(0) {}

        ShadowParams(const QPoint &offset, int radius, qreal opacity)
            : offset(offset)
            , radius(radius)
            , opacity(opacity) {}

        QPoint offset;
        int radius;
        qreal opacity;
    };

    struct CompositeShadowParams {
        CompositeShadowParams() = default;

        CompositeShadowParams(
                const QPoint &offset,
                const ShadowParams &shadow1,
                const ShadowParams &shadow2)
            : offset(offset)
            , shadow1(shadow1)
            , shadow2(shadow2) {}

        bool isNone() const {
            return qMax(shadow1.radius, shadow2.radius) == 0;
        }

        QPoint offset;
        ShadowParams shadow1;
        ShadowParams shadow2;
    };

    const CompositeShadowParams s_shadowParams[] = {
        // None
        CompositeShadowParams(),
        // Small
        CompositeShadowParams(
            QPoint(0, 12),
            ShadowParams(QPoint(0, 0), 16, 1),
            ShadowParams(QPoint(0, -6), 8, 0.4)),
        // Medium
        CompositeShadowParams(
            QPoint(0, 16),
            ShadowParams(QPoint(0, 0), 32, 0.9),
            ShadowParams(QPoint(0, -8), 16, 0.3)),
        // Large
        CompositeShadowParams(
            QPoint(0, 24),
            ShadowParams(QPoint(0, 0), 48, 0.8),
            ShadowParams(QPoint(0, -12), 24, 0.2)),
        // Very large
        CompositeShadowParams(
            QPoint(0, 48),
            ShadowParams(QPoint(0, 0), 64, 0.7),
            ShadowParams(QPoint(0, -24), 32, 0.1)),
    };

    inline CompositeShadowParams lookupShadowParams(int size)
    {
        switch (size) {
        case Breezeway::InternalSettings::ShadowNone:
            return s_shadowParams[0];
        case Breezeway::InternalSettings::ShadowSmall:
            return s_shadowParams[1];
        case Breezeway::InternalSettings::ShadowMedium:
            return s_shadowParams[2];
        case Breezeway::InternalSettings::ShadowLarge:
            return s_shadowParams[3];
        case Breezeway::InternalSettings::ShadowVeryLarge:
            return s_shadowParams[4];
        default:
            // Fallback to the Large size.
            return s_shadowParams[3];
        }
    }
}

namespace Breezeway
{

    using KDecoration2::ColorRole;
    using KDecoration2::ColorGroup;

    //________________________________________________________________
    static int g_sDecoCount = 0;
    static int g_shadowSizeEnum = InternalSettings::ShadowLarge;
    static int g_shadowStrength = 255;
    static QColor g_shadowColor = Qt::black;
    static QSharedPointer<KDecoration2::DecorationShadow> g_sActiveShadow;
    static QSharedPointer<KDecoration2::DecorationShadow> g_sInactiveShadow;

    //________________________________________________________________
    Decoration::Decoration(QObject *parent, const QVariantList &args)
        : KDecoration2::Decoration(parent, args)
        , m_animation( new QPropertyAnimation( this ) )
    {
        g_sDecoCount++;
    }

    //________________________________________________________________
    Decoration::~Decoration()
    {
        g_sDecoCount--;
        if (g_sDecoCount == 0) {
            // last deco destroyed, clean up shadow
            g_sActiveShadow.clear();
            g_sInactiveShadow.clear();
        }

        deleteSizeGrip();

    }

    //________________________________________________________________
    void Decoration::setOpacity( qreal value )
    {
        if( m_opacity == value ) return;
        m_opacity = value;
        update();

        if( m_sizeGrip ) m_sizeGrip->update();
    }

    //________________________________________________________________
    QColor Decoration::titleBarColor() const
    {
        auto c = client().data();
        if( hideTitleBar() ) return c->color( ColorGroup::Inactive, ColorRole::TitleBar );
        
        // if user specified a custom color per window
        else if ( customColorBoxEx() )
        {
            return m_internalSettings->customColorSelectEx();
        }
        // if users want to specify their own color values
        else if ( m_internalSettings->customColorBox() )
        {
            return m_internalSettings->customColorSelect();
        }
        // if one of the automatic settings is used
        else switch( m_internalSettings->matchTitleBarColor() )
        {
            default:
            case 0:
                // get titlebar color as specified by the color scheme
                if( m_animation->state() == QPropertyAnimation::Running )
                {
                    return KColorUtils::mix(
                        c->color( ColorGroup::Inactive, ColorRole::TitleBar ),
                        c->color( ColorGroup::Active, ColorRole::TitleBar ),
                        m_opacity );
                } else return c->color( c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::TitleBar );
            case 1:
                // use window color as titlebar color
                return c->palette().color(QPalette::Window);
        }
    }

    //________________________________________________________________
    QColor Decoration::outlineColor() const
    {
        auto c( client().data() );
        if( ( !m_internalSettings->drawTitleBarSeparator() && invertSeparator() ) || ( m_internalSettings->drawTitleBarSeparator() && !invertSeparator() ) ){
            if( c->isActive() ) return titleBarColor().lighter(70);
            else return titleBarColor().lighter(85);
        } else { 
            return QColor(); 
        }
    }

    //________________________________________________________________
    QColor Decoration::fontColor() const
    {
        const QRgb brightFont = 0xFFFFFFFF;
        QColor color = ( this->titleBarColor() );
        int y = 0.2126*color.red()+0.7152*color.green()+0.0722*color.blue();
        QColor newFont ( forceBrightFonts() ? brightFont : y > 128 ? color.lighter(40) : brightFont );
        auto c = client().data();
        if( m_animation->state() == QPropertyAnimation::Running )
        {
            if ( m_internalSettings->matchTitleBarColor() || customColorBoxEx() ){
                return KColorUtils::mix(
                    forceBrightFonts() ? color.lighter(140) : y > 128 ? newFont.lighter(140) : color.lighter(140),
                    newFont,
                    m_opacity );
            } else {
                return KColorUtils::mix(
                    c->color(ColorGroup::Inactive, ColorRole::Foreground ),
                    c->color(ColorGroup::Active, ColorRole::Foreground ),
                    m_opacity );
            }
        } else {
            if ( m_internalSettings->matchTitleBarColor() || customColorBoxEx() ){
                return c->isActive() ? newFont : forceBrightFonts() ? color.lighter(140) : y > 128 ? newFont.lighter(140) : color.lighter(140);
            } else {
                return  c->color( c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Foreground );

            }
        }

    }

    //________________________________________________________________
    void Decoration::init()
    {
        auto c = client().data();

        // active state change animation
        m_animation->setStartValue( 0 );
        m_animation->setEndValue( 1.0 );
        m_animation->setTargetObject( this );
        m_animation->setPropertyName( "opacity" );
        m_animation->setEasingCurve( QEasingCurve::InOutQuad );

        reconfigure();
        updateTitleBar();
        auto s = settings();
        connect(s.data(), &KDecoration2::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);

        // a change in font might cause the borders to change
        connect(s.data(), &KDecoration2::DecorationSettings::fontChanged, this, &Decoration::recalculateBorders);
        connect(s.data(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);

        // buttons
        connect(s.data(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::updateButtonsGeometryDelayed);
        connect(s.data(), &KDecoration2::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
        connect(s.data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);

        // full reconfiguration
        connect(s.data(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::reconfigure);
        connect(s.data(), &KDecoration2::DecorationSettings::reconfigured, SettingsProvider::self(), &SettingsProvider::reconfigure, Qt::UniqueConnection );
        connect(s.data(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

        connect(c, &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this, &Decoration::recalculateBorders);
        connect(c, &KDecoration2::DecoratedClient::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
        connect(c, &KDecoration2::DecoratedClient::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);
        connect(c, &KDecoration2::DecoratedClient::shadedChanged, this, &Decoration::recalculateBorders);
        connect(c, &KDecoration2::DecoratedClient::captionChanged, this,
            [this]()
            {
                // update the caption area
                update(titleBar());
            }
        );

        connect(c, &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateAnimationState);
        connect(c, &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateTitleBar);
        connect(c, &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleBar);
        connect(c, &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::setOpaque);

        connect(c, &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateButtonsGeometry);
        connect(c, &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateButtonsGeometry);
        connect(c, &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this, &Decoration::updateButtonsGeometry);
        connect(c, &KDecoration2::DecoratedClient::shadedChanged, this, &Decoration::updateButtonsGeometry);
 
        connect(c, &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::createShadow);

        createButtons();
        createShadow();
    }

    //________________________________________________________________
    void Decoration::updateTitleBar()
    {
        auto s = settings();
        auto c = client().data();
        const bool maximized = isMaximized();
        const int width =  maximized ? c->width() : c->width() - 2*s->largeSpacing()*customButtonMargin();
        const int height = maximized ? borderTop() : borderTop() - s->smallSpacing()*customTitleBarHeight();
        const int x = maximized ? 0 : s->largeSpacing()*customButtonMargin();
        const int y = maximized ? 0 : s->smallSpacing()*customTitleBarHeight();
        setTitleBar(QRect(x, y, width, height));
    }

    //________________________________________________________________
    void Decoration::updateAnimationState()
    {
        if( m_internalSettings->animationsEnabled() )
        {

            auto c = client().data();
            m_animation->setDirection( c->isActive() ? QPropertyAnimation::Forward : QPropertyAnimation::Backward );
            if( m_animation->state() != QPropertyAnimation::Running ) m_animation->start();

        } else {

            update();

        }
    }

    //________________________________________________________________
    void Decoration::updateSizeGripVisibility()
    {
        auto c = client().data();
        if( m_sizeGrip )
        { m_sizeGrip->setVisible( c->isResizeable() && !isMaximized() && !c->isShaded() ); }
    }

    //________________________________________________________________
    int Decoration::borderSize(bool bottom) const
    {
        const int baseSize = settings()->smallSpacing();
        if( m_internalSettings && (m_internalSettings->mask() & BorderSize ) )
        {
            switch (m_internalSettings->borderSize()) {
                case InternalSettings::BorderNone: return 0;
                case InternalSettings::BorderNoSides: return bottom ? qMax(4, baseSize) : 0;
                default:
                case InternalSettings::BorderTiny: return bottom ? qMax(4, baseSize) : baseSize;
                case InternalSettings::BorderNormal: return baseSize*2;
                case InternalSettings::BorderLarge: return baseSize*3;
                case InternalSettings::BorderVeryLarge: return baseSize*4;
                case InternalSettings::BorderHuge: return baseSize*5;
                case InternalSettings::BorderVeryHuge: return baseSize*6;
                case InternalSettings::BorderOversized: return baseSize*10;
            }

        } else {

            switch (settings()->borderSize()) {
                case KDecoration2::BorderSize::None: return 0;
                case KDecoration2::BorderSize::NoSides: return bottom ? qMax(4, baseSize) : 0;
                default:
                case KDecoration2::BorderSize::Tiny: return bottom ? qMax(4, baseSize) : baseSize;
                case KDecoration2::BorderSize::Normal: return baseSize*2;
                case KDecoration2::BorderSize::Large: return baseSize*3;
                case KDecoration2::BorderSize::VeryLarge: return baseSize*4;
                case KDecoration2::BorderSize::Huge: return baseSize*5;
                case KDecoration2::BorderSize::VeryHuge: return baseSize*6;
                case KDecoration2::BorderSize::Oversized: return baseSize*10;

            }

        }
    }

    //________________________________________________________________
    void Decoration::reconfigure()
    {

        m_internalSettings = SettingsProvider::self()->internalSettings( this );

        // animation
        m_animation->setDuration( m_internalSettings->animationsDuration() );

        // borders
        recalculateBorders();

        // shadow
        createShadow();

        // size grip
        if( hasNoBorders() && m_internalSettings->drawSizeGrip() ) createSizeGrip();
        else deleteSizeGrip();

    }

    //________________________________________________________________
    void Decoration::recalculateBorders()
    {
        auto c = client().data();
        auto s = settings();

        // left, right and bottom borders
        const int left   = isLeftEdge() ? 0 : borderSize();
        const int right  = isRightEdge() ? 0 : borderSize();
        const int bottom = (c->isShaded() || isBottomEdge()) ? 0 : borderSize(true);

        int top = 0;
        if( hideTitleBar() ) top = bottom;
        else {

            QFontMetrics fm(s->font());
            top += qMax(fm.height(), buttonHeight() );

            // padding below
            // extra pixel is used for the active window outline
            const int baseSize = s->smallSpacing();
            top += baseSize*customTitleBarHeight() + 1;

            // padding above
            top += baseSize*customTitleBarHeight();

        }

        setBorders(QMargins(left, top, right, bottom));

        // extended sizes
        const int extSize = s->largeSpacing();
        int extSides = 0;
        int extBottom = 0;
        if( hasNoBorders() )
        {
            extSides = extSize;
            extBottom = extSize;

        } else if( hasNoSideBorders() ) {

            extSides = extSize;

        }

        setResizeOnlyBorders(QMargins(extSides, 0, extSides, extBottom));
    }

    //________________________________________________________________
    void Decoration::createButtons()
    {
        m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &Button::create);
        m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &Button::create);
        updateButtonsGeometry();
    }

    //________________________________________________________________
    void Decoration::updateButtonsGeometryDelayed()
    { QTimer::singleShot( 0, this, &Decoration::updateButtonsGeometry ); }

    //________________________________________________________________
    void Decoration::updateButtonsGeometry()
    {
        const auto s = settings();

        // adjust button position
        const int bHeight = captionHeight() + (isTopEdge() ? s->smallSpacing()*customTitleBarHeight():0);
        const int bWidth = buttonHeight();
        const int verticalOffset = (isTopEdge() ? s->smallSpacing()*customTitleBarHeight():0) + (captionHeight()-buttonHeight())/2;
        foreach( const QPointer<KDecoration2::DecorationButton>& button, m_leftButtons->buttons() + m_rightButtons->buttons() )
        {
            button.data()->setGeometry( QRectF( QPoint( 0, 0 ), QSizeF( bWidth, bHeight ) ) );
            static_cast<Button*>( button.data() )->setOffset( QPointF( 0, verticalOffset ) );
            static_cast<Button*>( button.data() )->setIconSize( QSize( bWidth, bWidth ) );
        }

        // left buttons
        if( !m_leftButtons->buttons().isEmpty() )
        {

            // spacing
            m_leftButtons->setSpacing(s->smallSpacing()*customButtonSpacing());

            // padding
            const int vPadding = isTopEdge() ? 0 : s->smallSpacing()*customTitleBarHeight();
            const int hPadding = s->smallSpacing()*customButtonMargin();
            if( isLeftEdge() )
            {
                // add offsets on the side buttons, to preserve padding, but satisfy Fitts law
                auto button = static_cast<Button*>( m_leftButtons->buttons().front().data() );
                button->setGeometry( QRectF( QPoint( 0, 0 ), QSizeF( bWidth + hPadding, bHeight ) ) );
                button->setFlag( Button::FlagFirstInList );
                button->setHorizontalOffset( hPadding );

                m_leftButtons->setPos(QPointF(0, vPadding));

            } else m_leftButtons->setPos(QPointF(hPadding + borderLeft(), vPadding));

        }

        // right buttons
        if( !m_rightButtons->buttons().isEmpty() )
        {

            // spacing
            m_rightButtons->setSpacing(s->smallSpacing()*customButtonSpacing());

            // padding
            const int vPadding = isTopEdge() ? 0 : s->smallSpacing()*customTitleBarHeight();
            const int hPadding = s->smallSpacing()*customButtonMargin();
            if( isRightEdge() )
            {

                auto button = static_cast<Button*>( m_rightButtons->buttons().back().data() );
                button->setGeometry( QRectF( QPoint( 0, 0 ), QSizeF( bWidth + hPadding, bHeight ) ) );
                button->setFlag( Button::FlagLastInList );

                m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width(), vPadding));

            } else m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - hPadding - borderRight(), vPadding));

        }

        update();

    }

    //________________________________________________________________
    void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
    {
        // TODO: optimize based on repaintRegion
        auto c = client().data();
        auto s = settings();
        
        // paint background
        if( !c->isShaded() )
        {
            painter->fillRect(rect(), Qt::transparent);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush( c->color( c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Frame ) );

            // clip away the top part
            if( !hideTitleBar() ) painter->setClipRect(0, borderTop(), size().width(), size().height() - borderTop(), Qt::IntersectClip);

            if( s->isAlphaChannelSupported() ){
                painter->drawRoundedRect(rect(), customRadius(), customRadius());
            } else {
                painter->drawRect( rect() );
            } 
            painter->restore();
        }

        if( !hideTitleBar() ) paintTitleBar(painter, repaintRegion);

        if( hasBorders() && !s->isAlphaChannelSupported() )
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setBrush( Qt::NoBrush );
            painter->setPen( c->isActive() ?
                c->color( ColorGroup::Active, ColorRole::TitleBar ):
                c->color( ColorGroup::Inactive, ColorRole::Foreground ) );

            painter->drawRect( rect().adjusted( 0, 0, -1, -1 ) );
            painter->restore();
        }

        // draw highlight box
        // NOTE: box is drawn correctly along the edges but is
        // overlayed by the content of a window -> if 'no borders'
        // setting is used, this will make the line stop after
        // the titlebar, making it look unfinished
        if( drawHighlight() ){
            const QColor titleBarColor = (  this->titleBarColor() );
            const QRect windowRect( 
                QPoint(0, 0), 
                QSize( size().width(), size().height() ) );
            QColor sharpColor = titleBarColor.lighter(200);
            sharpColor.setAlpha(102);
            painter->setBrush( Qt::NoBrush );
            painter->setPen( sharpColor );
            painter->drawRoundedRect(windowRect, customRadius(), customRadius());
        }
    }

    //________________________________________________________________
    void Decoration::paintTitleBar(QPainter *painter, const QRect &repaintRegion)
    {
        const auto c = client().data();
        const QRect titleRect(QPoint(0, 0), QSize(size().width(), borderTop()));
        const QColor titleBarColor = (  this->titleBarColor() );

        if ( !titleRect.intersects(repaintRegion) ) return;

        painter->save();
        painter->setPen(Qt::NoPen);

        // render a linear gradient on titlebar including highlight area
        if( ( m_internalSettings->drawBackgroundGradient() && !invertGradient() ) || ( !m_internalSettings->drawBackgroundGradient() && invertGradient() ) )
        {
            QColor color = ( this->titleBarColor() );
            int y = 0.2126*color.red()+0.7152*color.green()+0.0722*color.blue();
            const int lfv = y > 128? 104: 110;
            const int gv = y > 128? 95: 90;
            const int lightfactor = c->isActive()? lfv: 100;
            QLinearGradient gradient( 0, 0, 0, titleRect.height() );
            gradient.setColorAt(0.0, titleBarColor.lighter(185) );
            gradient.setColorAt(0.04, titleBarColor.lighter(lightfactor));
            gradient.setColorAt(0.8, titleBarColor.lighter(gv));
            painter->setBrush(gradient);

        // if user doesn't want a gradient, we only paint highlight line
        // and titlebar color
        } else {
            QLinearGradient gradient(0, 0, 0, titleRect.height());
            gradient.setColorAt(0.0, titleBarColor.lighter(185));
            gradient.setColorAt(0.04, titleBarColor);
            painter->setBrush(gradient);

        }

        auto s = settings();
        if( isMaximized() || !s->isAlphaChannelSupported() )
        {

            painter->drawRect(titleRect);

        } else if( c->isShaded() ) {

            painter->drawRoundedRect(titleRect, customRadius(), customRadius());

        } else {

            painter->setClipRect(titleRect, Qt::IntersectClip);

            // the rect is made a little bit larger to be able to clip away the rounded corners at the bottom and sides
            // NOTE: this is possibly the reason for the shadow
            // not properly attaching to the bottom of a titlebar when
            // collapsing the window and leaving a blank space below it
            painter->drawRoundedRect(titleRect.adjusted(
                isLeftEdge() ? -customRadius():0,
                isTopEdge() ? -customRadius():0,
                isRightEdge() ? customRadius():0,
                customRadius()),
                customRadius(), customRadius());

        }

        const QColor outlineColor( this->outlineColor() );
        if( !c->isShaded() && outlineColor.isValid() )
        {
            // titlebar separator line
            painter->setRenderHint( QPainter::Antialiasing, false );
            painter->setBrush( Qt::NoBrush );
            painter->setPen( outlineColor );
            painter->drawLine( titleRect.bottomLeft(), titleRect.bottomRight() );
        }

        painter->restore();

        // draw caption
        painter->setFont(s->font());
        painter->setPen( fontColor() );
        const auto cR = captionRect();
        const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.first.width());
        painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);

        // draw all buttons
        m_leftButtons->paint(painter, repaintRegion);
        m_rightButtons->paint(painter, repaintRegion);

    }

    //________________________________________________________________
    int Decoration::customTitleBarHeight() const
    {
        switch( m_internalSettings->titleBarHeight() )
        {
            case 0: return Metrics::TitleBar_MarginSmall;
            default:
            case 1: return Metrics::TitleBar_Margin;
            case 2: return Metrics::TitleBar_MarginLarge;
        }
    }

    //________________________________________________________________
    int Decoration::customButtonMargin() const
    {
        switch( m_internalSettings->buttonMargin() )
        {
            case 0: return Metrics::TitleBar_SideMarginSmall;
            default:
            case 1: return Metrics::TitleBar_SideMargin;
            case 2: return Metrics::TitleBar_SideMarginLarge;
        }
    }

    //________________________________________________________________
    int Decoration::customButtonSpacing() const
    {
        switch( m_internalSettings->buttonSpacing() )
        {
            case 0: return Metrics::TitleBar_ButtonSpacingSmall;
            default:
            case 1: return Metrics::TitleBar_ButtonSpacing;
            case 2: return Metrics::TitleBar_ButtonSpacingLarge;
        }
    }

    //________________________________________________________________
    int Decoration::customRadius() const
    {
        switch( m_internalSettings->borderRadius() )
        {
            case 0: return Metrics::Frame_FrameRadiusNone;
            case 1: return Metrics::Frame_FrameRadiusTiny;
            default:
            case 2: return Metrics::Frame_FrameRadius;
            case 3: return Metrics::Frame_FrameRadiusExtended;
        }
    }

    //________________________________________________________________
    int Decoration::buttonHeight() const
    {
        const int baseSize = settings()->gridUnit();
        // baseSize is roughly 8 pixel
        switch( m_internalSettings->buttonSize() )
        {
            case InternalSettings::ButtonTiny: return baseSize;
            case InternalSettings::ButtonSmall: return baseSize*1.5;
            default:
            case InternalSettings::ButtonNormal: return baseSize*2-2;
            case InternalSettings::ButtonLarge: return baseSize*2.5;
            case InternalSettings::ButtonVeryLarge: return baseSize*3.5;
        }

    }

    //________________________________________________________________
    int Decoration::captionHeight() const
    { return hideTitleBar() ? borderTop() : borderTop() - settings()->smallSpacing()*(customTitleBarHeight() + customTitleBarHeight() ) - 1; }

    //________________________________________________________________
    QPair<QRect,Qt::Alignment> Decoration::captionRect() const
    {
        if( hideTitleBar() ) return qMakePair( QRect(), Qt::AlignCenter );
        else {

            auto c = client().data();
            const int leftOffset = m_leftButtons->buttons().isEmpty() ?
                customButtonMargin()*settings()->smallSpacing():
                m_leftButtons->geometry().x() + m_leftButtons->geometry().width() + customButtonMargin()*settings()->smallSpacing();

            const int rightOffset = m_rightButtons->buttons().isEmpty() ?
                customButtonMargin()*settings()->smallSpacing() :
                size().width() - m_rightButtons->geometry().x() + customButtonMargin()*settings()->smallSpacing();

         
            const int yOffset = settings()->smallSpacing()*customTitleBarHeight();
            const QRect maxRect( leftOffset, yOffset, size().width() - leftOffset - rightOffset, captionHeight() );

            switch( m_internalSettings->titleAlignment() )
            {
                case InternalSettings::AlignLeft:
                return qMakePair( maxRect, Qt::AlignVCenter|Qt::AlignLeft );

                case InternalSettings::AlignRight:
                return qMakePair( maxRect, Qt::AlignVCenter|Qt::AlignRight );

                case InternalSettings::AlignCenter:
                return qMakePair( maxRect, Qt::AlignCenter );

                default:
                case InternalSettings::AlignCenterFullWidth:
                {

                    // full caption rect
                    const QRect fullRect = QRect( 0, yOffset, size().width(), captionHeight() );
                    QRect boundingRect( settings()->fontMetrics().boundingRect( c->caption()).toRect() );

                    // text bounding rect
                    boundingRect.setTop( yOffset );
                    boundingRect.setHeight( captionHeight() );
                    boundingRect.moveLeft( ( size().width() - boundingRect.width() )/2 );

                    if( boundingRect.left() < leftOffset ) return qMakePair( maxRect, Qt::AlignVCenter|Qt::AlignLeft );
                    else if( boundingRect.right() > size().width() - rightOffset ) return qMakePair( maxRect, Qt::AlignVCenter|Qt::AlignRight );
                    else return qMakePair(fullRect, Qt::AlignCenter);

                }

            }

        }

    }

    //________________________________________________________________
    void Decoration::createShadow()
    {
        QSharedPointer<KDecoration2::DecorationShadow> sShadow;
        auto c = client().data();
        if ( c->isActive() ) {
            sShadow = g_sActiveShadow;
        } else {
            sShadow = g_sInactiveShadow;
        }

        if (!sShadow
                ||g_shadowSizeEnum != m_internalSettings->shadowSize()
                || g_shadowStrength != m_internalSettings->shadowStrength()
                || g_shadowColor != m_internalSettings->shadowColor())
        {
            g_shadowSizeEnum = m_internalSettings->shadowSize();
            g_shadowStrength = m_internalSettings->shadowStrength();
            g_shadowColor = m_internalSettings->shadowColor();


            const CompositeShadowParams params = lookupShadowParams(g_shadowSizeEnum);
            if (params.isNone()) {
                sShadow.clear();
                setShadow(sShadow);
                return;
            }

            auto withOpacity = [](const QColor &color, qreal opacity) -> QColor {
                QColor c(color);
                c.setAlphaF(opacity);
                return c;
            };

            const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
                .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));


            BoxShadowRenderer shadowRenderer;
            shadowRenderer.setBorderRadius(customRadius() + 0.5);
            shadowRenderer.setBoxSize(boxSize);
            shadowRenderer.setDevicePixelRatio(1.0); // TODO: Create HiDPI shadows?

            qreal strength = static_cast<qreal>(g_shadowStrength) / 255.0;

            // set shadow strength values for inactive windows
            if ( !c->isActive() ) {
                if(m_internalSettings->inactiveShadowBehaviour() == 0){
                    strength /= 2;
                } else if (m_internalSettings->inactiveShadowBehaviour() == 1){
                    strength /= 4;
                } else if (m_internalSettings->inactiveShadowBehaviour() == 2){
                    strength = 0;
                }
            }
            shadowRenderer.addShadow(params.shadow1.offset, params.shadow1.radius,
                withOpacity(g_shadowColor, params.shadow1.opacity * strength));
            shadowRenderer.addShadow(params.shadow2.offset, params.shadow2.radius,
                withOpacity(g_shadowColor, params.shadow2.opacity * strength));

            QImage shadowTexture = shadowRenderer.render();

            QPainter painter(&shadowTexture);
            painter.setRenderHint(QPainter::Antialiasing);

            const QRect outerRect = shadowTexture.rect();

            QRect boxRect(QPoint(0, 0), boxSize);
            boxRect.moveCenter(outerRect.center());

            const QRect titleRect(QPoint(0, 0), QSize(size().width(), borderTop()));

            // Mask out inner rect.
            const QMargins padding = QMargins(
                boxRect.left() - outerRect.left() - Metrics::Shadow_Overlap - params.offset.x(),
                boxRect.top() - outerRect.top() - Metrics::Shadow_Overlap - params.offset.y(),
                outerRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x(),
                outerRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y());
            const QRect innerRect = outerRect - padding;

            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::black);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            painter.drawRoundedRect(
                innerRect,
                customRadius() + 0.5,
                customRadius() + 0.5);
            

            // Dirty hack to draw things inside the frame
            // NOTE: this is currently rendered underneath
            // the window as it is part of the shadow construct
            const QMargins b_padding = QMargins(
                boxRect.left() - outerRect.left() - Metrics:: Shadow_Overlap - params.offset.x() + 1,
                boxRect.top() - outerRect.top() - Metrics::Shadow_Overlap - params.offset.y() + 1,
                outerRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x() + 1,
                outerRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y() + 1);
            const QRect overRect = outerRect - b_padding; // test and see what this draws

            // this is the basecolor for the highlight
            static QColor b_highlight = Qt::white;
            // setting up the stroke color
            painter.setPen(withOpacity(b_highlight.lighter(120), 0.2));
            // setting up brush
            painter.setBrush(Qt::NoBrush);
            // setting up composition mode
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            // creating area to draw
            painter.drawRoundedRect(
                overRect, // responsible for the size of the rect
                customRadius() - 0.5,
                customRadius() - 0.5);

            // Draw outline.
            painter.setPen(withOpacity(g_shadowColor, 0.6 * strength));
            painter.setBrush(Qt::NoBrush);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawRoundedRect(
                innerRect,
                customRadius() - 0.5,
                customRadius() - 0.5);

            painter.end();

            sShadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
            sShadow->setPadding(padding);
            sShadow->setInnerShadowRect(QRect(outerRect.center(), QSize(1, 1)));
            sShadow->setShadow(shadowTexture);
        }

        setShadow(sShadow);
    }

    //_________________________________________________________________
    void Decoration::createSizeGrip()
    {

        // do nothing if size grip already exist
        if( m_sizeGrip ) return;

        #if BREEZEWAY_HAVE_X11
        if( !QX11Info::isPlatformX11() ) return;

        // access client
        auto c = client().data();
        if( !c ) return;

        if( c->windowId() != 0 )
        {
            m_sizeGrip = new SizeGrip( this );
            connect( c, &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateSizeGripVisibility );
            connect( c, &KDecoration2::DecoratedClient::shadedChanged, this, &Decoration::updateSizeGripVisibility );
            connect( c, &KDecoration2::DecoratedClient::resizeableChanged, this, &Decoration::updateSizeGripVisibility );
        }
        #endif

    }

    //_________________________________________________________________
    void Decoration::deleteSizeGrip()
    {
        if( m_sizeGrip )
        {
            m_sizeGrip->deleteLater();
            m_sizeGrip = nullptr;
        }
    }

} // namespace


#include "breezewaydecoration.moc"
