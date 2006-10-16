/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2003   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_TEXT_ENGINE_H
#define QWT_TEXT_ENGINE_H 1

#include <qsize.h>
#include "qwt_global.h"

class QFont;
class QRect;
class QString;
class QPainter;

/*! 
  \brief Abstract base class for rendering text strings

  A text engine is responsible for rendering texts for a
  specific text format. They are used by QwtText to render a text. 

  QwtPlainTextEngine and QwtRichTextEngine
  are part of the Qwt library, other text engines can be loaded 
  as plugins.

  The Qt Solution package includes a renderer for MathML, that could
  be used successfully for an implementation of a MathML text plugin. 
  Unfortunately the QtMml implementation needs a lot of patches to 
  export interfaces to be usable from Qwt, that can't be published 
  without violation of the Qt Solution license. 
  ( See textengines/mathml, and read the comments in mathml.pro. )

  \sa QwtText
*/

class QWT_EXPORT QwtTextEngine
{
public:
    virtual ~QwtTextEngine();

    /*!
      Find the height for a given width

      \param font Font of the text
      \param flags Bitwise OR of the flags used like in QPainter::drawText
      \param text Text to be rendered
      \param width Width 

      \return Calculated height
     */
    virtual int heightForWidth(const QFont &font, int flags, 
        const QString &text, int width) const = 0;

    /*!
      Returns the size, that is needed to render text

      \param font Font of the text
      \param flags Bitwise OR of the flags like in for QPainter::drawText
      \param text Text to be rendered

      \return Caluclated size
     */
    virtual QSize textSize(const QFont &font, int flags,
        const QString &text) const = 0;

    /*! 
      Test if a string can be rendered by this text engine

      \param text Text to be tested
      \return true, if it can be rendered
     */
    virtual bool mightRender(const QString &text) const = 0;

    /*!
      Return margins around the texts

      The textSize might include margins around the 
      text, like QFontMetrics::descent. In situations
      where texts need to be aligend in detail, knowing
      these margins might improve the layout calculations.

      \param font Font of the text
      \param text Text to be rendered
      \param left Return value for the left margin
      \param right Return value for the right margin
      \param top Return value for the top margin
      \param bottom Return value for the bottom margin
     */
    virtual void textMargins(const QFont &font, const QString &text,
        int &left, int &right, int &top, int &bottom) const = 0;

    /*!
      Draw the text in a clipping rectangle

      \param painter Painter
      \param rect Clipping rectangle
      \param flags Bitwise OR of the flags like in for QPainter::drawText
      \param text Text to be rendered
     */ 
    virtual void draw(QPainter *painter, const QRect &rect,
        int flags, const QString &text) const = 0;

protected:
    QwtTextEngine();
};


/*!
  \brief A text engine for plain texts

  QwtPlainTextEngine renders texts using the basic Qt classes
  QPainter and QFontMetrics. 
*/
class QWT_EXPORT QwtPlainTextEngine: public QwtTextEngine
{
public:
    QwtPlainTextEngine();
    virtual ~QwtPlainTextEngine();

    virtual int heightForWidth(const QFont &font, int flags, 
        const QString &text, int width) const;

    virtual QSize textSize(const QFont &font, int flags,
        const QString &text) const;

    virtual void draw(QPainter *painter, const QRect &rect,
        int flags, const QString &text) const;

    virtual bool mightRender(const QString &) const;

    virtual void textMargins(const QFont &, const QString &,
        int &left, int &right, int &top, int &bottom) const;

private:
    class PrivateData; 
    PrivateData *d_data;
};


#ifndef QT_NO_RICHTEXT

/*!
  \brief A text engine for Qt rich texts

  QwtRichTextEngine renders Qt rich texts using the classes
  of the Scribe framework of Qt.
*/
class QWT_EXPORT QwtRichTextEngine: public QwtTextEngine
{
public:
    QwtRichTextEngine();

    virtual int heightForWidth(const QFont &font, int flags, 
        const QString &text, int width) const;

    virtual QSize textSize(const QFont &font, int flags,
        const QString &text) const;

    virtual void draw(QPainter *painter, const QRect &rect,
        int flags, const QString &text) const;

    virtual bool mightRender(const QString &) const;

    virtual void textMargins(const QFont &, const QString &,
        int &left, int &right, int &top, int &bottom) const;
private:
    QString taggedText(const QString &, int flags) const;
};

#endif // !QT_NO_RICHTEXT

#endif