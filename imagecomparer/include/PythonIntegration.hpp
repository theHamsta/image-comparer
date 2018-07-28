/**
 * \file   PythonIntegration.hpp
 * \brief  Defines class PythonIntegration
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   16.09.2017
 */

#ifndef PYTHONINTEGRATION_HPP
#define PYTHONINTEGRATION_HPP

#include <boost/filesystem.hpp>
#include "pybind11/embed.h"
#include <mutex>


namespace py = pybind11;
class PythonIntegration
{
	public:
		virtual ~PythonIntegration()
		{
			s_instance = nullptr;
		}

		static inline PythonIntegration* instance()
		{
			if ( !s_instance )
				s_instance = new PythonIntegration();

			return s_instance;
		}

		PythonIntegration( const PythonIntegration& copy ) = delete;

		std::vector<py::module>& modules() { return m_modules; }
		static std::mutex& mutex();

		void import_path( const boost::filesystem::path& path );
		void import_module( const boost::filesystem::path& pythonFile );
		void exec_commands( const std::string& commands );
		// boost::python::object getObject( const std::string& objname ) { return m_mainNamespace[objname.c_str()]; }

	protected:
		std::vector<py::module> m_modules;

	private:
		PythonIntegration();

		static PythonIntegration* s_instance;
		py::scoped_interpreter m_interpreter{};
};


#endif /* end of include guard: PYTHONINTEGRATION_HPP */

