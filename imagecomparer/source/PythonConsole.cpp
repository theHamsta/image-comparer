#include "PythonConsole.hpp"

#include <QTextEdit>
#include <QLayout>
#include <QBoxLayout>
#include <QDebug>

#include <QFontDatabase>
#include "PythonIntegration.hpp"

#pragma push_macro("slots")
#undef slots
#include "pybind11/eval.h"
#include <pybind11/iostream.h>
#pragma pop_macro("slots")


PythonConsole::PythonConsole( QWidget* parent ):
	QWidget( parent )
{
	setLayout( new QBoxLayout( QBoxLayout::Down ) ) ;
	// setGeometry( 0, 0, 400, 300 );
	const QFont fixedFont = QFontDatabase::systemFont( QFontDatabase::FixedFont );

	m_currentTextEdit = new QTextEdit();
	m_currentTextEdit->setFont(	fixedFont );
	m_currentTextEdit->zoomIn( 2 );

	m_outputTextEdit = new QTextEdit();
	m_outputTextEdit->setReadOnly( true );
	m_outputTextEdit->setFont(	fixedFont );
	m_outputTextEdit->zoomIn( 2 );
	m_outputTextEdit->setAlignment( Qt::AlignBottom );

	layout()->addWidget( m_outputTextEdit );
	layout()->addWidget( m_currentTextEdit );
	connect( m_currentTextEdit, &QTextEdit::textChanged, this, &PythonConsole::onTextChanged );
	m_currentTextEdit->setFocus();

	setWindowTitle( tr( "Python Console" ) );
}

void PythonConsole::onTextChanged()
{

	QString text = m_currentTextEdit->toPlainText();

	if ( text.contains( "\n" ) ) {
		// auto locals = py::dict( "path"_a = path.string() );
		m_outputTextEdit->setPlainText( m_outputTextEdit->toPlainText() + m_currentTextEdit->toPlainText() );

		try {

			py::exec( text.toStdString(), py::globals() );
		} catch ( const std::exception& e ) {
			qDebug() << QString::fromStdString( e.what() );
			m_outputTextEdit->setPlainText( m_outputTextEdit->toPlainText() + QString::fromStdString( e.what() ) + "\n" );

		}

		m_lastCommand = m_currentTextEdit->toPlainText();

		m_currentTextEdit->setPlainText( "" );



	}
}
