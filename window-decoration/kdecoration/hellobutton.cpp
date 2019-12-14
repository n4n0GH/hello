/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
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
#include "hellobutton.h"

#include <KDecoration2/DecoratedClient>
#include <KColorUtils>

#include <QPainter>

namespace Hello
{

    using KDecoration2::ColorRole;
    using KDecoration2::ColorGroup;
    using KDecoration2::DecorationButtonType;


    //__________________________________________________________________
    Button::Button(DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
        , m_animation( new QPropertyAnimation( this ) )
    {

        // setup animation
        m_animation->setStartValue( 0 );
        m_animation->setEndValue( 1.0 );
        m_animation->setTargetObject( this );
        m_animation->setPropertyName( "opacity" );
        m_animation->setEasingCurve( QEasingCurve::InOutQuad );

        // setup default geometry
        const int height = decoration->buttonHeight();
        setGeometry(QRect(0, 0, height, height));
        setIconSize(QSize( height, height ));

        // connections
        connect(decoration->client().data(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
        connect(decoration->settings().data(), &KDecoration2::DecorationSettings::reconfigured, this, &Button::reconfigure);
        connect( this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState );
        if (decoration->objectName() == "applet-window-buttons") {
            connect(this, &Button::hoveredChanged, [=](bool hovered){
                decoration->setButtonHovered(hovered);
            });
        }
        connect(decoration, &Decoration::buttonHoveredChanged, [&](){ 
            update();
        });

        reconfigure();

    }

    //__________________________________________________________________
    Button::Button(QObject *parent, const QVariantList &args)
        : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
    {
        m_flag = FlagStandalone;
        //! icon size must return to !valid because it was altered from the default constructor,
        //! in Standalone mode the button is not using the decoration metrics but its geometry
        m_iconSize = QSize(-1, -1);
    }
            
    //__________________________________________________________________
    Button *Button::create(DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
    {
        if (auto d = qobject_cast<Decoration*>(decoration))
        {
            Button *b = new Button(type, d, parent);
            switch( type )
            {

                case DecorationButtonType::Close:
                b->setVisible( d->client().data()->isCloseable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::closeableChanged, b, &Hello::Button::setVisible );
                break;

                case DecorationButtonType::Maximize:
                b->setVisible( d->client().data() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Hello::Button::setVisible );
                break;

                case DecorationButtonType::Minimize:
                b->setVisible( d->client().data()->isMinimizeable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::minimizeableChanged, b, &Hello::Button::setVisible );
                break;

                case DecorationButtonType::ContextHelp:
                b->setVisible( d->client().data()->providesContextHelp() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::providesContextHelpChanged, b, &Hello::Button::setVisible );
                break;

                case DecorationButtonType::Shade:
                b->setVisible( d->client().data()->isShadeable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::shadeableChanged, b, &Hello::Button::setVisible );
                break;

                case DecorationButtonType::Menu:
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() { b->update(); });
                break;

                default: break;

            }

            return b;
        }

        return nullptr;

    }

    //__________________________________________________________________
    void Button::paint(QPainter *painter, const QRect &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        if (!decoration()) return;

        painter->save();

        // translate from offset
        if( m_flag == FlagFirstInList ) painter->translate( m_offset );
        else painter->translate( 0, m_offset.y() );

        if( !m_iconSize.isValid() ) m_iconSize = geometry().size().toSize();

        // menu button
        if (type() == DecorationButtonType::Menu)
        {

            const QRectF iconRect( geometry().topLeft(), m_iconSize );
            decoration()->client().data()->icon().paint(painter, iconRect.toRect());


        } else {

            drawIcon( painter );

        }

        painter->restore();

    }

    //__________________________________________________________________
    void Button::drawIcon( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
        scale painter so that its window matches QRect( -1, -1, 20, 20 )
        this makes all further rendering and scaling simpler
        all further rendering is preformed inside QRect( 0, 0, 18, 18 )
        */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );

        // render background
        const QColor backgroundColor( this->backgroundColor() );
        if( backgroundColor.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( backgroundColor.lighter(85) );
            painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
            painter->setBrush( backgroundColor );
            painter->drawEllipse( QRectF( 1, 1, 16, 16 ) );
        }

        // render mark
        const QColor foregroundColor( this->foregroundColor() );
        if( foregroundColor.isValid() )
        {

            // setup painter
            QPen pen( foregroundColor );
            pen.setCapStyle( Qt::RoundCap );
            pen.setJoinStyle( Qt::RoundJoin );
            pen.setWidthF( 1.3*qMax((qreal)1.0, 20/width ) );

            painter->setPen( pen );
            painter->setBrush( foregroundColor );

            switch( type() )
            {

                case DecorationButtonType::Close:
                {
                    painter->drawLine( 6, 6, 12, 12 );
                    painter->drawLine( 12, 6, 6, 12 );
                    break;
                }

                case DecorationButtonType::Maximize:
                {
                    if( isChecked() )
                    {
                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 10, 5 ),
                            QPointF( 10, 8 ),
                            QPointF( 13, 8 )} );
                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 5, 10 ),
                            QPointF( 8, 10 ),
                            QPointF( 8, 13 )} );
                    } else {
                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 9, 6 ),
                            QPointF( 12, 6 ),
                            QPointF( 12, 9 )} );
                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 9, 12 ),
                            QPointF( 6, 12 ),
                            QPointF( 6, 9 )} );
                    }
                    break;
                }

                case DecorationButtonType::Minimize:
                {
                    painter->drawLine( 5, 9, 13, 9 );
                    break;
                }

                case DecorationButtonType::OnAllDesktops:
                {
                    painter->setPen( Qt::NoPen );
                    painter->setBrush( foregroundColor );

                    if( isChecked())
                    {

                        // outer ring
                        painter->drawEllipse( QRectF( 3, 3, 12, 12 ) );

                        // center dot
                        QColor backgroundColor( colorSymbol );
                        auto d = qobject_cast<Decoration*>( decoration() );
                        if( !backgroundColor.isValid() && d ) backgroundColor = d->titleBarColor();

                        if( backgroundColor.isValid() )
                        {
                            painter->setBrush( backgroundColor );
                            painter->drawEllipse( QRectF( 5, 5, 8, 8 ) );
                        }

                    } else {

                        painter->drawEllipse( QRectF( 7, 7, 4, 4 ) );
                    }
                    break;
                }

                case DecorationButtonType::Shade:
                {

                    if (isChecked())
                    {
                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 6, 7 ),
                            QPointF( 9, 4 ),
                            QPointF( 12, 7 )} );
                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 6, 11 ),
                            QPointF( 12, 11 ),
                            QPointF( 9, 14 )} );

                    } else {
                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 6, 4 ),
                            QPointF( 12, 4 ),
                            QPointF( 9, 7 )} );
                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 6, 14 ),
                            QPointF( 9, 11 ),
                            QPointF( 12, 14 )} );
                    }

                    break;

                }

                case DecorationButtonType::KeepBelow:
                {
                    painter->drawPolygon( QVector<QPointF>{
                        QPointF( 6, 6 ),
                        QPointF( 12, 6 ),
                        QPointF( 9, 9 ) 
                    });
                    painter->drawPolyline( QVector<QPointF> {
                        QPointF( 6, 10 ),
                        QPointF( 9, 13 ),
                        QPointF( 12, 10)
                    });
                    break;

                }

                case DecorationButtonType::KeepAbove:
                {
                    painter->drawPolygon( QVector<QPointF>{
                        QPointF( 6, 12 ),
                        QPointF( 9, 9 ),
                        QPointF( 12, 12 )} );
                    painter->drawPolyline( QVector<QPointF> {
                        QPointF( 6, 8 ),
                        QPointF( 9, 5 ),
                        QPointF( 12, 8)
                    });
                    break;
                }


                case DecorationButtonType::ApplicationMenu:
                {
                    painter->drawLine( 5, 6, 13, 6 );
                    painter->drawLine( 5, 9, 13, 9 );
                    painter->drawLine( 5, 12, 13, 12 );
                    break;
                }

                case DecorationButtonType::ContextHelp:
                {
                    pen.setJoinStyle( Qt::BevelJoin );
                    pen.setCapStyle( Qt::SquareCap );
                    // eyes
                    painter->drawLine(6, 6, 6, 8);
                    painter->drawLine(13, 6, 13, 8);
                    // nose
                    painter->drawPolyline( QVector<QPointF> {
                        QPointF( 10, 7),
                        QPointF( 10, 10),
                        QPointF( 9, 10)
                    });
                    // mouth
                    painter->drawPoint(6, 12);
                    painter->drawPoint(13, 12);
                    painter->drawLine(7, 13, 12, 13);
                    pen.setJoinStyle( Qt::RoundJoin );
                    pen.setCapStyle( Qt::RoundCap );
                    break;
                }

                default: break;

            }

        }

    }

    //__________________________________________________________________
    QColor Button::foregroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        auto c = d->client().data();
        auto s = d->internalSettings()->buttonCustomColor();
        QColor customCloseColor = d->internalSettings()->customCloseColor();
        QColor customMinColor = d->internalSettings()->customMinColor();
        QColor customMaxColor = d->internalSettings()->customMaxColor();
        QColor customShadeColor = d->internalSettings()->customShadeColor();
        QColor customOtherColor = d->internalSettings()->customOtherColor();
        QColor customAboveColor = d->internalSettings()->customAboveColor();
        QColor customBelowColor = d->internalSettings()->customBelowColor();
        QColor customMenuColor = d->internalSettings()->customMenuColor();
        QColor customPinColor = d->internalSettings()->customPinColor();

        auto f = d->internalSettings()->forceBrightFonts();
        auto buttonIcons = d->internalSettings()->buttonIconsBox();
        bool hovered = isHovered() || d->buttonHovered();

        if( !d || buttonIcons == 4 ) {

            return QColor();

        } else if( isPressed() && buttonIcons != 4 ) {

            return QColor(colorSymbol);
        } else if( 
            ( 
                type() == DecorationButtonType::KeepBelow 
                || type() == DecorationButtonType::KeepAbove 
                || type() == DecorationButtonType::Shade 
                || type() == DecorationButtonType::OnAllDesktops 
            ) && isChecked() ){

            return d->titleBarColor();

        } else if( 
            (buttonIcons == 2 || 
            buttonIcons == 3 || 
            ( buttonIcons == 0 ? isHovered() : hovered )) && buttonIcons != 4 ){

            if ( c->isActive() && buttonIcons != 3 ){
                QColor color;
                if( type() == DecorationButtonType::Close ) {
                    if(c->isCloseable() ){
                        if(s){ color.setRgb(colorClose);
                        } else { color = customCloseColor; }
                    }
                } else if( type() == DecorationButtonType::Maximize ) {
                    if(c->isMaximizeable() ){
                        if(s){ color.setRgb(colorMaximize);
                        } else { color = customMaxColor; }
                    }
                } else if( type() == DecorationButtonType::Minimize ) {
                    if(c->isMinimizeable() ){
                        if(s){ color.setRgb(colorMinimize);
                        } else { color = customMinColor; }
                    }
                } else if( type() == DecorationButtonType::Shade ) {
                    if(s){ color.setRgb(colorOther);
                    } else { color = customShadeColor; }
                } else if( type() == DecorationButtonType::KeepBelow ) {
                    if(s){ color.setRgb(colorOther);
                    } else { color = customBelowColor; }
                } else if( type() == DecorationButtonType::KeepAbove ) {
                    if(s){ color.setRgb(colorOther);
                    } else { color = customAboveColor; }
                } else if( type() == DecorationButtonType::OnAllDesktops ) {
                    if(s){ color.setRgb(colorOther);
                    } else { color = customPinColor; }
                } else if( type() == DecorationButtonType::ApplicationMenu ) {
                    if(s){ color.setRgb(colorOther);
                    } else { color = customMenuColor; }
                } else {
                    if(s){ color.setRgb(colorOther);
                    } else { color = customOtherColor; }
                }
                if( !s ) {
                    int y = (0.2126*color.red())+(0.7152*color.green())+(0.0722*color.blue());
                    if ( y > 128 ) { return color.lighter(40); 
                    } else if ( y == 0 ) { return Qt::white;
                    } else { return color.lighter(240); }
                } else { return color.lighter(40); }

            } else if ( c->isActive() && buttonIcons == 3 ){

                QColor color;
                color = d->titleBarColor();
                int y = (0.2126*color.red())+(0.7152*color.green())+(0.0722*color.blue());
                QColor iconColor ( f ? Qt::white : y >= 128 ? color.lighter(40) : Qt::white );
                return iconColor;

            } else if ( !c->isActive() && buttonIcons == 3) {
        
                QColor color;
                color = d->titleBarColor();
                int y = (0.2126*color.red())+(0.7152*color.green())+(0.0722*color.blue());
                QColor newColor ( f ? Qt::white : y > 128 ? color.lighter(40) : Qt::white );
                return KColorUtils::mix(
                    f ? color.lighter(140) : y > 128 ? newColor.lighter(140) : color.lighter(140),
                    Qt::white,
                    m_opacity );

            } else {
                QColor color;
                color = d->titleBarColor();
                return color.lighter(60);
            }

        } else if( m_animation->state() == QPropertyAnimation::Running ) {
            
            auto c = d->client().data();
            if ( !c->isActive() ) {
                QColor color;
                color = d->titleBarColor();
                color.setAlpha(255*m_opacity);
                return color.lighter(60);
            }

            QColor color;
            if( type() == DecorationButtonType::Close ) {
                if(s){ color.setRgb(colorClose);
                } else { color = customCloseColor; }
            } else if( type() == DecorationButtonType::Maximize ) {
                if(s){ color.setRgb(colorMaximize);
                } else { color = customMaxColor; }
            } else if( type() == DecorationButtonType::Minimize ) {
                if(s){ color.setRgb(colorMinimize);
                } else { color = customMinColor; }
            } else if( type() == DecorationButtonType::Shade ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customShadeColor; }
            } else if( type() == DecorationButtonType::KeepBelow ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customBelowColor; }
            } else if( type() == DecorationButtonType::KeepAbove ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customAboveColor; }
            } else if( type() == DecorationButtonType::OnAllDesktops ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customPinColor; }
            } else if( type() == DecorationButtonType::ApplicationMenu ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customMenuColor; }
            } else {
                if(s){ color.setRgb(colorOther);
                } else { color = customOtherColor; }
            }
            return KColorUtils::mix( color, color.lighter(60), m_opacity );

        } 
        // else if( isHovered() ) {

        //     return QColor(colorSymbol);

        // } 
        else {

            return backgroundColor();

        }

    }

    //__________________________________________________________________
    QColor Button::backgroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        

        if( !d ) {

            return QColor();

        }

        auto c = d->client().data();
        auto buttonIcons = d->internalSettings()->buttonIconsBox();
        auto s = d->internalSettings()->buttonCustomColor();
        QColor customCloseColor = d->internalSettings()->customCloseColor();
        QColor customMinColor = d->internalSettings()->customMinColor();
        QColor customMaxColor = d->internalSettings()->customMaxColor();
        QColor customShadeColor = d->internalSettings()->customShadeColor();
        QColor customOtherColor = d->internalSettings()->customOtherColor();
        QColor customAboveColor = d->internalSettings()->customAboveColor();
        QColor customBelowColor = d->internalSettings()->customBelowColor();
        QColor customMenuColor = d->internalSettings()->customMenuColor();
        QColor customPinColor = d->internalSettings()->customPinColor();

        QColor luma;
        QColor color;
        color = d->titleBarColor();
        int y = 0.2126*color.red()+0.7152*color.green()+0.0722*color.blue();
        if(y > 128) {
            luma = color.lighter(85);
        } else {
            luma = color.lighter(145);
        }

        if( isPressed() && buttonIcons != 3 ) {

            QColor color;
            if( type() == DecorationButtonType::Close ) {
                if(s){ color.setRgb(colorClose); 
                } else { color = customCloseColor; }
            } else if( type() == DecorationButtonType::Maximize ) {
                if(s){ color.setRgb(colorMaximize); 
                } else { color = customMaxColor; }
            } else if( type() == DecorationButtonType::Minimize ) {
                if(s){ color.setRgb(colorMinimize);
                } else { color = customMinColor; }
            } else if( type() == DecorationButtonType::Shade ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customShadeColor; }
            } else if( type() == DecorationButtonType::KeepBelow ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customBelowColor; }
            } else if( type() == DecorationButtonType::KeepAbove ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customAboveColor; }
            } else if( type() == DecorationButtonType::OnAllDesktops ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customPinColor; }
            } else if( type() == DecorationButtonType::ApplicationMenu ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customMenuColor; }
            } else {
                if(s){ color.setRgb(colorOther);
                } else { color = customOtherColor; }
            }
            return KColorUtils::mix( color, QColor(colorSymbol), 0.3 );

        } else if( isPressed() && buttonIcons == 3 ) {

            return QColor();

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade || type() == DecorationButtonType::OnAllDesktops ) && isChecked() ) {

            return d->fontColor();

        } else if ( !c->isActive() && buttonIcons != 3 ) {

            return luma;

        } else if ( !c->isActive() && buttonIcons == 3 ) {

            return QColor();

        } else if ( c->isActive() && buttonIcons == 3 ) {

            return QColor();

        } else {

            QColor color;
            if( type() == DecorationButtonType::Close ) {
                if(!c->isCloseable()){
                    return luma;
                } else {
                    if(s) { color.setRgb(colorClose);
                    } else { color = customCloseColor; }
                }
            } else if( type() == DecorationButtonType::Maximize ) {
                if(!c->isMaximizeable()){
                    return luma;
                } else {
                    if(s){ color.setRgb(colorMaximize);                  
                    } else { color = customMaxColor; }
                }
            } else if( type() == DecorationButtonType::Minimize ) {
                if(!c->isMinimizeable()){
                    return luma;
                } else {
                    if(s){ color.setRgb(colorMinimize);
                    } else { color = customMinColor; }
                }
            // we can stop checking if the buttons are xable, because these will
            // not be rendered if the functionality doesn't apply to the window
            } else if( type() == DecorationButtonType::Shade ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customShadeColor; }
            }
            else if( type() == DecorationButtonType::KeepAbove ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customAboveColor; }
            }
            else if( type() == DecorationButtonType::KeepBelow ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customBelowColor; }
            }
            else if( type() == DecorationButtonType::OnAllDesktops ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customPinColor; }
            }
            else if( type() == DecorationButtonType::ApplicationMenu ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customMenuColor; }
            }
            else if( type() == DecorationButtonType::ContextHelp ) {
                if(s){ color.setRgb(colorOther);
                } else { color = customOtherColor; }
            }
            return color;

        }

    }

    //________________________________________________________________
    void Button::reconfigure()
    {

        // animation
        auto d = qobject_cast<Decoration*>(decoration());
        if( d )  m_animation->setDuration( d->internalSettings()->animationsDuration() );

    }

    //__________________________________________________________________
    void Button::updateAnimationState( bool hovered )
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if( !(d && d->internalSettings()->animationsEnabled() ) ) return;

        m_animation->setDirection( hovered ? QPropertyAnimation::Forward : QPropertyAnimation::Backward );
        if( m_animation->state() != QPropertyAnimation::Running ) m_animation->start();

    }

} // namespace
