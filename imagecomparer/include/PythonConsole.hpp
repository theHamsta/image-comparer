#ifndef PYTHONCONSOLE_HPP
#define PYTHONCONSOLE_HPP
#include <QWidget>

class QTextEdit;

class PythonConsole : public QWidget
{
		Q_OBJECT

	public:
		explicit PythonConsole( QWidget* parent );

	private:
		QTextEdit* m_currentTextEdit;
		QTextEdit* m_outputTextEdit;
		QString m_lastCommand;
	private slots:
		void onTextChanged();
};

#endif /* PYTHONCONSOLE_HPP */
