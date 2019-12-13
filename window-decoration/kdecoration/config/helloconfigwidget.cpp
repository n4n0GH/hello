//////////////////////////////////////////////////////////////////////////////
// helloconfigurationui.cpp
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "helloconfigwidget.h"
#include "helloexceptionlist.h"
#include "hellosettings.h"

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>
// TODO: clean up the connectors and group them like they appear within the UI
namespace Hello
{

    //_________________________________________________________
    ConfigWidget::ConfigWidget( QWidget* parent, const QVariantList &args ):
        KCModule(parent, args),
        m_configuration( KSharedConfig::openConfig( QStringLiteral( "hellorc" ) ) ),
        m_changed( false )
    {

        // configuration
        m_ui.setupUi( this );

        // track ui changes
        connect( m_ui.titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        // connect( m_ui.outlineCloseButton, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawBorderOnMaximizedWindows, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawSizeGrip, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawBackgroundGradient, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawTitleBarSeparator, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.matchTitleBarColor, SIGNAL(currentIndexChanged(int)), SLOT( updateChanged()) );
        connect( m_ui.customColorBox, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.customColorSelect, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.drawHighlight, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawTitleHighlight, SIGNAL(clicked()), SLOT(updateChanged()) );

        // titlebar settings
        connect( m_ui.titleBarHeightSpin, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );

        // border radius settings
        connect( m_ui.borderRadiusSpin, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );

        // custom button settings
        connect( m_ui.buttonSizeSpin, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonSpacingSpin, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonMarginSpin, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonIconsBox, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );

        // use custom color for buttons or not
        connect( m_ui.buttonCustomColor, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.customCloseColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.customMinColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.customMaxColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.customShadeColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.customOtherColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.customAboveColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.customBelowColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.customPinColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.customMenuColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );

        // track animations changes
        connect( m_ui.animationsEnabled, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.animationsDuration, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );

        // track shadows changes
        connect( m_ui.shadowSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.shadowStrength, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.shadowColor, SIGNAL(changed(QColor)), SLOT(updateChanged()) );
        connect( m_ui.inactiveShadowBehaviour, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );

        // track exception changes
        connect( m_ui.exceptions, SIGNAL(changed(bool)), SLOT(updateChanged()) );

    }

    //_________________________________________________________
    void ConfigWidget::load()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->load();

        // assign to ui
        m_ui.titleAlignment->setCurrentIndex( m_internalSettings->titleAlignment() );
        m_ui.drawBorderOnMaximizedWindows->setChecked( m_internalSettings->drawBorderOnMaximizedWindows() );
        // m_ui.outlineCloseButton->setChecked( m_internalSettings->outlineCloseButton() );
        m_ui.drawSizeGrip->setChecked( m_internalSettings->drawSizeGrip() );
        m_ui.drawBackgroundGradient->setChecked( m_internalSettings->drawBackgroundGradient() );
        m_ui.animationsEnabled->setChecked( m_internalSettings->animationsEnabled() );
        m_ui.animationsDuration->setValue( m_internalSettings->animationsDuration() );
        m_ui.drawTitleBarSeparator->setChecked( m_internalSettings->drawTitleBarSeparator() );
        m_ui.matchTitleBarColor->setCurrentIndex( m_internalSettings->matchTitleBarColor() );
        m_ui.customColorBox->setChecked( m_internalSettings->customColorBox() );
        m_ui.customColorSelect->setColor( m_internalSettings->customColorSelect() );
        m_ui.drawHighlight->setChecked( m_internalSettings->drawHighlight() );
        m_ui.drawTitleHighlight->setChecked( m_internalSettings->drawTitleHighlight() );
        m_ui.buttonSizeSpin->setValue( qreal(m_internalSettings->buttonSizeSpin()) );
        m_ui.buttonSpacingSpin->setValue( qreal(m_internalSettings->buttonSpacingSpin()) );
        m_ui.buttonMarginSpin->setValue( qreal(m_internalSettings->buttonMarginSpin()) );
        m_ui.titleBarHeightSpin->setValue( qreal(m_internalSettings->titleBarHeightSpin()) );
        m_ui.borderRadiusSpin->setValue( qreal(m_internalSettings->borderRadiusSpin()) );

        m_ui.buttonCustomColor->setChecked( m_internalSettings->buttonCustomColor() );
        m_ui.customCloseColor->setColor( m_internalSettings->customCloseColor() );
        m_ui.customMinColor->setColor( m_internalSettings->customMinColor() );
        m_ui.customMaxColor->setColor( m_internalSettings->customMaxColor() );
        m_ui.customShadeColor->setColor( m_internalSettings->customShadeColor() );
        m_ui.customOtherColor->setColor( m_internalSettings->customOtherColor() );
        m_ui.customAboveColor->setColor( m_internalSettings->customAboveColor() );
        m_ui.customBelowColor->setColor( m_internalSettings->customBelowColor() );
        m_ui.customPinColor->setColor( m_internalSettings->customPinColor() );
        m_ui.customMenuColor->setColor( m_internalSettings->customMenuColor() );

        m_ui.buttonIconsBox->setCurrentIndex( m_internalSettings->buttonIconsBox() );

        // load shadows
        if( m_internalSettings->shadowSize() <= InternalSettings::ShadowVeryLarge ) m_ui.shadowSize->setCurrentIndex( m_internalSettings->shadowSize() );
        else m_ui.shadowSize->setCurrentIndex( InternalSettings::ShadowLarge );

        m_ui.shadowStrength->setValue( qRound(qreal(m_internalSettings->shadowStrength()*100)/255 ) );
        m_ui.shadowColor->setColor( m_internalSettings->shadowColor() );

        m_ui.inactiveShadowBehaviour->setCurrentIndex( m_internalSettings->inactiveShadowBehaviour() );

        // load exceptions
        ExceptionList exceptions;
        exceptions.readConfig( m_configuration );
        m_ui.exceptions->setExceptions( exceptions.get() );
        setChanged( false );

    }

    //_________________________________________________________
    void ConfigWidget::save()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->load();

        // apply modifications from ui
        m_internalSettings->setTitleAlignment( m_ui.titleAlignment->currentIndex() );
        // m_internalSettings->setOutlineCloseButton( m_ui.outlineCloseButton->isChecked() );
        m_internalSettings->setDrawBorderOnMaximizedWindows( m_ui.drawBorderOnMaximizedWindows->isChecked() );
        m_internalSettings->setDrawSizeGrip( m_ui.drawSizeGrip->isChecked() );
        m_internalSettings->setDrawBackgroundGradient( m_ui.drawBackgroundGradient->isChecked() );
        m_internalSettings->setAnimationsEnabled( m_ui.animationsEnabled->isChecked() );
        m_internalSettings->setAnimationsDuration( m_ui.animationsDuration->value() );
        m_internalSettings->setDrawTitleBarSeparator(m_ui.drawTitleBarSeparator->isChecked() );
        m_internalSettings->setMatchTitleBarColor( m_ui.matchTitleBarColor->currentIndex() );
        m_internalSettings->setCustomColorBox( m_ui.customColorBox->isChecked() );
        m_internalSettings->setCustomColorSelect( m_ui.customColorSelect->color() );
        m_internalSettings->setDrawHighlight( m_ui.drawHighlight->isChecked() );
        m_internalSettings->setDrawTitleHighlight( m_ui.drawTitleHighlight->isChecked() );
        m_internalSettings->setButtonSizeSpin( qreal(m_ui.buttonSizeSpin->value()) );
        m_internalSettings->setButtonSpacingSpin( qreal(m_ui.buttonSpacingSpin->value()) );
        m_internalSettings->setButtonMarginSpin( qreal(m_ui.buttonMarginSpin->value()) );
        m_internalSettings->setTitleBarHeightSpin( qreal(m_ui.titleBarHeightSpin->value()) );
        m_internalSettings->setBorderRadiusSpin( qreal(m_ui.borderRadiusSpin->value()) );

        m_internalSettings->setButtonCustomColor( m_ui.buttonCustomColor->isChecked() );
        m_internalSettings->setCustomCloseColor( m_ui.customCloseColor->color() );
        m_internalSettings->setCustomMinColor( m_ui.customMinColor->color() );
        m_internalSettings->setCustomMaxColor( m_ui.customMaxColor->color() );
        m_internalSettings->setCustomShadeColor( m_ui.customShadeColor->color() );
        m_internalSettings->setCustomOtherColor( m_ui.customOtherColor->color() );
        m_internalSettings->setCustomBelowColor( m_ui.customBelowColor->color() );
        m_internalSettings->setCustomAboveColor( m_ui.customAboveColor->color() );
        m_internalSettings->setCustomPinColor( m_ui.customPinColor->color() );
        m_internalSettings->setCustomMenuColor( m_ui.customMenuColor->color() );

        m_internalSettings->setButtonIconsBox( m_ui.buttonIconsBox->currentIndex() );

        m_internalSettings->setShadowSize( m_ui.shadowSize->currentIndex() );
        m_internalSettings->setShadowStrength( qRound( qreal(m_ui.shadowStrength->value()*255)/100 ) );
        m_internalSettings->setShadowColor( m_ui.shadowColor->color() );

        m_internalSettings->setInactiveShadowBehaviour( m_ui.inactiveShadowBehaviour->currentIndex() );

        // save configuration
        m_internalSettings->save();

        // get list of exceptions and write
        InternalSettingsList exceptions( m_ui.exceptions->exceptions() );
        ExceptionList( exceptions ).writeConfig( m_configuration );

        // sync configuration
        m_configuration->sync();
        setChanged( false );

        // needed to tell kwin to reload when running from external kcmshell
        {
            QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
            QDBusConnection::sessionBus().send(message);
        }

        // needed for hello style to reload shadows
        {
            QDBusMessage message( QDBusMessage::createSignal("/HelloDecoration",  "org.kde.Hello.Style", "reparseConfiguration") );
            QDBusConnection::sessionBus().send(message);
        }

    }

    //_________________________________________________________
    void ConfigWidget::defaults()
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->setDefaults();

        // assign to ui
        m_ui.titleAlignment->setCurrentIndex( m_internalSettings->titleAlignment() );
        m_ui.drawBorderOnMaximizedWindows->setChecked( m_internalSettings->drawBorderOnMaximizedWindows() );
        m_ui.drawSizeGrip->setChecked( m_internalSettings->drawSizeGrip() );
        m_ui.drawBackgroundGradient->setChecked( m_internalSettings->drawBackgroundGradient() );
        m_ui.animationsEnabled->setChecked( m_internalSettings->animationsEnabled() );
        m_ui.animationsDuration->setValue( m_internalSettings->animationsDuration() );
        m_ui.drawTitleBarSeparator->setChecked( m_internalSettings->drawTitleBarSeparator() );
        m_ui.matchTitleBarColor->setCurrentIndex( m_internalSettings->matchTitleBarColor() );
        m_ui.customColorBox->setChecked( m_internalSettings->customColorBox() );
        m_ui.customColorSelect->setColor( m_internalSettings->customColorSelect() );
        m_ui.drawHighlight->setChecked( m_internalSettings->drawHighlight() );
        m_ui.drawTitleHighlight->setChecked( m_internalSettings->drawTitleHighlight() );
        m_ui.buttonSizeSpin->setValue( qreal(m_internalSettings->buttonSizeSpin()) );
        m_ui.buttonSpacingSpin->setValue( qreal(m_internalSettings->buttonSpacingSpin()) );
        m_ui.buttonMarginSpin->setValue( qreal(m_internalSettings->buttonMarginSpin()) );
        m_ui.titleBarHeightSpin->setValue( qreal(m_internalSettings->titleBarHeightSpin()) );
        m_ui.borderRadiusSpin->setValue( qreal(m_internalSettings->borderRadiusSpin()) );

        m_ui.buttonCustomColor->setChecked( m_internalSettings->buttonCustomColor() );
        m_ui.customCloseColor->setColor( m_internalSettings->customCloseColor() );
        m_ui.customMinColor->setColor( m_internalSettings->customMinColor() );
        m_ui.customMaxColor->setColor( m_internalSettings->customMaxColor() );
        m_ui.customShadeColor->setColor( m_internalSettings->customShadeColor() );
        m_ui.customOtherColor->setColor( m_internalSettings->customOtherColor() );
        m_ui.customAboveColor->setColor( m_internalSettings->customAboveColor() );
        m_ui.customBelowColor->setColor( m_internalSettings->customBelowColor() );
        m_ui.customPinColor->setColor( m_internalSettings->customPinColor() );
        m_ui.customMenuColor->setColor( m_internalSettings->customMenuColor() );

        m_ui.buttonIconsBox->setCurrentIndex( m_internalSettings->buttonIconsBox() );

        m_ui.shadowSize->setCurrentIndex( m_internalSettings->shadowSize() );
        m_ui.shadowStrength->setValue( qRound(qreal(m_internalSettings->shadowStrength()*100)/255 ) );
        m_ui.shadowColor->setColor( m_internalSettings->shadowColor() );

        m_ui.inactiveShadowBehaviour->setCurrentIndex( m_internalSettings->inactiveShadowBehaviour() );

    }

    //_______________________________________________
    void ConfigWidget::updateChanged()
    {

        // check configuration
        if( !m_internalSettings ) return;

        // track modifications
        bool modified( false );

        if (m_ui.drawTitleBarSeparator->isChecked() != m_internalSettings->drawTitleBarSeparator()) modified = true;
        if( m_ui.titleAlignment->currentIndex() != m_internalSettings->titleAlignment() ) modified = true;
        // else if( m_ui.outlineCloseButton->isChecked() != m_internalSettings->outlineCloseButton() ) modified = true;
        else if( m_ui.drawBorderOnMaximizedWindows->isChecked() !=  m_internalSettings->drawBorderOnMaximizedWindows() ) modified = true;
        else if( m_ui.drawSizeGrip->isChecked() !=  m_internalSettings->drawSizeGrip() ) modified = true;
        else if( m_ui.drawBackgroundGradient->isChecked() !=  m_internalSettings->drawBackgroundGradient() ) modified = true;
        else if( m_ui.matchTitleBarColor->currentIndex() != m_internalSettings->matchTitleBarColor() ) modified = true;
        else if( m_ui.customColorBox->isChecked() != m_internalSettings->customColorBox() ) modified = true;
        else if( m_ui.customColorSelect->color() != m_internalSettings->customColorSelect() ) modified = true;
        else if( m_ui.drawHighlight->isChecked() != m_internalSettings->drawHighlight() ) modified = true;
        else if( m_ui.drawTitleHighlight->isChecked() != m_internalSettings->drawTitleHighlight() ) modified = true;
        else if( qreal(m_ui.buttonSizeSpin->value() ) != m_internalSettings->buttonSizeSpin() ) modified = true;
        else if( qreal(m_ui.buttonSpacingSpin->value() ) != m_internalSettings->buttonSpacingSpin() ) modified = true;
        else if( qreal(m_ui.buttonMarginSpin->value() ) != m_internalSettings->buttonMarginSpin() ) modified = true;
        else if( qreal(m_ui.titleBarHeightSpin->value() ) != m_internalSettings->titleBarHeightSpin() ) modified = true;
        else if( qreal(m_ui.borderRadiusSpin->value() ) != m_internalSettings->borderRadiusSpin() ) modified = true;

        // custom button colors
        else if( m_ui.buttonCustomColor->isChecked() != m_internalSettings->buttonCustomColor() ) modified = true;
        else if( m_ui.customCloseColor->color() != m_internalSettings->customCloseColor() ) modified = true;
        else if( m_ui.customMinColor->color() != m_internalSettings->customMinColor() ) modified = true;
        else if( m_ui.customMaxColor->color() != m_internalSettings->customMaxColor() ) modified = true;
        else if( m_ui.customShadeColor->color() != m_internalSettings->customShadeColor() ) modified = true;
        else if( m_ui.customOtherColor->color() != m_internalSettings->customOtherColor() ) modified = true;
        else if( m_ui.customAboveColor->color() != m_internalSettings->customAboveColor() ) modified = true;
        else if( m_ui.customBelowColor->color() != m_internalSettings->customBelowColor() ) modified = true;
        else if( m_ui.customPinColor->color() != m_internalSettings->customPinColor() ) modified = true;
        else if( m_ui.customMenuColor->color() != m_internalSettings->customMenuColor() ) modified = true;

        else if( m_ui.buttonIconsBox->currentIndex() != m_internalSettings->buttonIconsBox() ) modified = true;

        // animations
        else if( m_ui.animationsEnabled->isChecked() !=  m_internalSettings->animationsEnabled() ) modified = true;
        else if( m_ui.animationsDuration->value() != m_internalSettings->animationsDuration() ) modified = true;

        // shadows
        else if( m_ui.shadowSize->currentIndex() !=  m_internalSettings->shadowSize() ) modified = true;
        else if( qRound( qreal(m_ui.shadowStrength->value()*255)/100 ) != m_internalSettings->shadowStrength() ) modified = true;
        else if( m_ui.shadowColor->color() != m_internalSettings->shadowColor() ) modified = true;
        else if( m_ui.inactiveShadowBehaviour->currentIndex() != m_internalSettings->inactiveShadowBehaviour() ) modified = true;

        // exceptions
        else if( m_ui.exceptions->isChanged() ) modified = true;

        setChanged( modified );

    }

    //_______________________________________________
    void ConfigWidget::setChanged( bool value )
    {
        emit changed( value );
    }

}
