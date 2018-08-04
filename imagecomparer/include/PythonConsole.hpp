#ifndef PYTHONCONSOLE_HPP
#define PYTHONCONSOLE_HPP

#include <QScrollArea>

class QTextEdit;

class PythonConsole : public QWidget
{
		Q_OBJECT

	public:
		explicit PythonConsole( QWidget* parent );

	private:
		QScrollArea* m_qScrollArea;
		QTextEdit* m_currentTextEdit;
		QTextEdit* m_outputTextEdit;
		QString m_lastCommand;
	private slots:
		void onTextChanged();
};

#endif /* PYTHONCONSOLE_HPP */
