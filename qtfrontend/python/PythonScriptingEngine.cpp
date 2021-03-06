/***************************************************************************
	File                 : PythonScriptingEngine.cpp
	Project              : SciDAVis
--------------------------------------------------------------------
	Copyright            : (C) 2006,2008 by Knut Franke
	Email (use @ for *)  : knut.franke*gmx.de
	Description          : Execute Python code from within SciDAVis

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
// get rid of a compiler warning
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#include <Python.h>
#include <compile.h>
#include <eval.h>
#include <frameobject.h>
#include <traceback.h>

#if PY_VERSION_HEX < 0x020400A1
typedef struct _traceback {
	PyObject_HEAD
		struct _traceback *tb_next;
	PyFrameObject *tb_frame;
	int tb_lasti;
	int tb_lineno;
} PyTracebackObject;
#endif

#include "PythonScript.h"
#include "PythonScriptingEngine.h"

#include <QObject>
#include <QStringList>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>

// includes sip.h, which undefines Qt's "slots" macro since SIP 4.6
// TODO
//#include "sipAPIscidavis.h"
//extern "C" void initscidavis();

const char* PythonScriptingEngine::g_lang_name = "Python";

QString PythonScriptingEngine::toString(PyObject *object, bool decref)
{
	QString ret;
	if (!object) return "";
	PyObject *repr = PyObject_Str(object);
	if (decref) Py_DECREF(object);
	if (!repr) return "";
	ret = PyString_AsString(repr);
	Py_DECREF(repr);
	return ret;
}

PyObject *PythonScriptingEngine::eval(const QString &code, PyObject *argDict, const char *name)
{
	PyObject *args;
	if (argDict)
	{
		Py_INCREF(argDict);
		args = argDict;
	} else
		args = PyDict_New();
	PyObject *ret=NULL;
	PyObject *co = Py_CompileString(code.toAscii().constData(), name, Py_eval_input);
	if (co)
	{
		ret = PyEval_EvalCode((PyCodeObject*)co, m_globals, args);
		Py_DECREF(co);
	}
	Py_DECREF(args);
	return ret;
}

bool PythonScriptingEngine::exec (const QString &code, PyObject *argDict, const char *name)
{
	PyObject *args;
	if (argDict)
	{
		Py_INCREF(argDict);
		args = argDict;
	} else
		args = PyDict_New();
	PyObject *tmp = NULL;
	PyObject *co = Py_CompileString(code.toAscii().constData(), name, Py_file_input);
	if (co)
	{
		tmp = PyEval_EvalCode((PyCodeObject*)co, m_globals, args);
		Py_DECREF(co);
	}
	Py_DECREF(args);
	if (!tmp) return false;
	Py_DECREF(tmp);
	return true;
}

QString PythonScriptingEngine::errorMsg()
{
	PyObject *exception=0, *value=0, *traceback=0;
	PyTracebackObject *excit=0;
	PyFrameObject *frame;
	char *fname;
	QString msg;
	if (!PyErr_Occurred()) return "";

	PyErr_Fetch(&exception, &value, &traceback);
	PyErr_NormalizeException(&exception, &value, &traceback);
	if(PyErr_GivenExceptionMatches(exception, PyExc_SyntaxError))
	{
		QString text = toString(PyObject_GetAttrString(value, "text"), true);
		msg.append(text + "\n");
		PyObject *offset = PyObject_GetAttrString(value, "offset");
		for (int i=0; i<(PyInt_AsLong(offset)-1); i++)
			if (text[i] == '\t')
				msg.append("\t");
			else
				msg.append(" ");
		msg.append("^\n");
		Py_DECREF(offset);
		msg.append("SyntaxError: ");
		msg.append(toString(PyObject_GetAttrString(value, "msg"), true) + "\n");
		msg.append("at ").append(toString(PyObject_GetAttrString(value, "filename"), true));
		msg.append(":").append(toString(PyObject_GetAttrString(value, "lineno"), true));
		msg.append("\n");
		Py_DECREF(exception);
		Py_DECREF(value);
	} else {
		msg.append(toString(exception,true)).remove("exceptions.").append(": ");
		msg.append(toString(value,true));
		msg.append("\n");
	}

	if (traceback) {
		excit = (PyTracebackObject*)traceback;
		while (excit && (PyObject*)excit != Py_None)
		{
			frame = excit->tb_frame;
			msg.append("at ").append(PyString_AsString(frame->f_code->co_filename));
			msg.append(":").append(QString::number(excit->tb_lineno));
			if (frame->f_code->co_name && *(fname = PyString_AsString(frame->f_code->co_name)) != '?')
				msg.append(" in ").append(fname);
			msg.append("\n");
			excit = excit->tb_next;
		}
		Py_DECREF(traceback);
	}

	return msg;
}

PythonScriptingEngine::PythonScriptingEngine()
	: AbstractScriptingEngine(g_lang_name)
{
	m_globals = 0;
	m_math = 0;
	m_sys = 0;
}

void PythonScriptingEngine::initialize()
{
	PyObject *mainmod=NULL, *scidavismod=NULL, *sysmod=NULL;
	if (Py_IsInitialized())
	{
//		PyEval_AcquireLock();
		mainmod = PyImport_ImportModule("__main__");
		if (!mainmod)
		{
			PyErr_Print();
//			PyEval_ReleaseLock();
			return;
		}
		m_globals = PyModule_GetDict(mainmod);
		Py_DECREF(mainmod);
	} else {
//		PyEval_InitThreads ();
		Py_Initialize ();
		if (!Py_IsInitialized ())
			return;
		//TODO: initscidavis();

		mainmod = PyImport_AddModule("__main__");
		if (!mainmod)
		{
//			PyEval_ReleaseLock();
			PyErr_Print();
			return;
		}
		m_globals = PyModule_GetDict(mainmod);
	}

	if (!m_globals)
	{
		PyErr_Print();
//		PyEval_ReleaseLock();
		return;
	}
	Py_INCREF(m_globals);

	m_math = PyDict_New();
	if (!m_math)
		PyErr_Print();

	/* TODO
	scidavismod = PyImport_ImportModule("scidavis");
	if (scidavismod)
	{
		PyDict_SetItemString(globals, "scidavis", scidavismod);
		PyObject *scidavisDict = PyModule_GetDict(scidavismod);
		setQObject(m_parent, "app", scidavisDict);
		PyDict_SetItemString(scidavisDict, "mathFunctions", math);
		Py_DECREF(scidavismod);
	} else
		PyErr_Print();
		*/

	sysmod = PyImport_ImportModule("sys");
	if (sysmod)
	{
		m_sys = PyModule_GetDict(sysmod);
		Py_INCREF(m_sys);
	} else
		PyErr_Print();

	// Redirect output to the print(const QString&) signal.
	// Also see method write(const QString&) and Python documentation on
	// sys.stdout and sys.stderr.
	setQObject(this, "stdout", m_sys);
	setQObject(this, "stderr", m_sys);

#ifdef Q_WS_WIN
	loadInitFile(QDir::homePath()+"/scidavisrc") ||
		loadInitFile(QCoreApplication::instance()->applicationDirPath()+"/scidavisrc") ||
#else
	loadInitFile(QDir::homePath()+"/.scidavisrc") ||
		loadInitFile(QDir::rootPath()+"etc/scidavisrc") ||
#endif
		loadInitFile("scidavisrc");

	m_initialized = true;

//	PyEval_ReleaseLock();
}

PythonScriptingEngine::~PythonScriptingEngine()
{
	Py_XDECREF(m_globals);
	Py_XDECREF(m_math);
	Py_XDECREF(m_sys);
}

bool PythonScriptingEngine::loadInitFile(const QString &path)
{
	QFileInfo pyFile(path+".py"), pycFile(path+".pyc");
	bool success = false;
	if (pycFile.isReadable() && (pycFile.lastModified() >= pyFile.lastModified())) {
		// if we have a recent pycFile, use it
		FILE *f = fopen(pycFile.filePath().toAscii(), "rb");
		success = PyRun_SimpleFileEx(f, pycFile.filePath().toAscii(), false) == 0;
		fclose(f);
	} else if (pyFile.isReadable() && pyFile.exists()) {
		// try to compile pyFile to pycFile
		PyObject *compileModule = PyImport_ImportModule("py_compile");
		if (compileModule) {
			PyObject *compile = PyDict_GetItemString(PyModule_GetDict(compileModule), "compile");
			if (compile) {
				PyObject *tmp = PyObject_CallFunctionObjArgs(compile,
						PyString_FromString(pyFile.filePath().toAscii()),
						PyString_FromString(pycFile.filePath().toAscii()),
						NULL);
				if (tmp)
					Py_DECREF(tmp);
				else
					PyErr_Print();
			} else
				PyErr_Print();
			Py_DECREF(compileModule);
		} else
			PyErr_Print();
		pycFile.refresh();
		if (pycFile.isReadable() && (pycFile.lastModified() >= pyFile.lastModified())) {
			// run the newly compiled pycFile
			FILE *f = fopen(pycFile.filePath().toAscii(), "rb");
			success = PyRun_SimpleFileEx(f, pycFile.filePath().toAscii(), false) == 0;
			fclose(f);
		} else {
			// fallback: just run pyFile
			/*FILE *f = fopen(pyFile.filePath(), "r");
			success = PyRun_SimpleFileEx(f, pyFile.filePath(), false) == 0;
			fclose(f);*/
			//TODO: code above crashes on Windows - bug in Python?
			QFile f(pyFile.filePath());
			if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QByteArray data = f.readAll();
				success = PyRun_SimpleString(data.data());
				f.close();
			}
		}
	}
	return success;
}

bool PythonScriptingEngine::isRunning() const
{
	return Py_IsInitialized();
}

bool PythonScriptingEngine::setQObject(QObject *val, const char *name, PyObject *dict)
{
	if(!val) return false;
	PyObject *pyobj=NULL;
	/* TODO
	sipTypeDef *t;
	for (int i=0; i<sipModuleAPI_scidavis.em_nrtypes; i++)
			// Note that the SIP API is a bit confusing here.
			// sipTypeDef.td_cname holds the C++ class name, but is NULL if that's the same as the Python class name.
			// sipTypeDef.td_name OTOH always holds the Python class name, but prepended by the module name ("scidavis.")
			if (((t=sipModuleAPI_scidavis.em_types[i]->type)->td_cname && !strcmp(val->metaObject()->className(),t->td_cname)) ||
					(!t->td_cname && !strcmp(val->metaObject()->className(),t->td_name+9)))
			{
				pyobj=sipConvertFromInstance(val,sipModuleAPI_scidavis.em_types[i],NULL);
				if (!pyobj) return false;
				break;
			}
	if (!pyobj) {
		for (int i=0; i<sipModuleAPI_scidavis_QtCore->em_nrtypes; i++)
				if (((t=sipModuleAPI_scidavis_QtCore->em_types[i]->type)->td_cname && !strcmp(val->metaObject()->className(),t->td_cname)) ||
						(!t->td_cname && !strcmp(val->metaObject()->className(),t->td_name+3)))
				{
					pyobj=sipConvertFromInstance(val,sipModuleAPI_scidavis_QtCore->em_types[i],NULL);
					if (!pyobj) return false;
					break;
				}
	} 
	*/
	if (!pyobj) return false;

	if (dict)
		PyDict_SetItemString(dict, name, pyobj);
	else
		PyDict_SetItemString(m_globals, name, pyobj);
	Py_DECREF(pyobj);
	return true;
}

bool PythonScriptingEngine::setInt(int val, const char *name, PyObject *dict)
{
	PyObject *pyobj = Py_BuildValue("i",val);
	if (!pyobj) return false;
	if (dict)
		PyDict_SetItemString(dict, name, pyobj);
	else
		PyDict_SetItemString(m_globals, name, pyobj);
	Py_DECREF(pyobj);
	return true;
}

bool PythonScriptingEngine::setDouble(double val, const char *name, PyObject *dict)
{
	PyObject *pyobj = Py_BuildValue("d",val);
	if (!pyobj) return false;
	if (dict)
		PyDict_SetItemString(dict, name, pyobj);
	else
		PyDict_SetItemString(m_globals, name, pyobj);
	Py_DECREF(pyobj);
	return true;
}

const QStringList PythonScriptingEngine::mathFunctions() const
{
	QStringList flist;
	PyObject *key, *value;
#if PY_VERSION_HEX >= 0x02050000
	Py_ssize_t i=0;
#else
	int i=0;
#endif
	while(PyDict_Next(m_math, &i, &key, &value))
		if (PyCallable_Check(value))
			flist << PyString_AsString(key);
	flist.sort();
	return flist;
}

const QString PythonScriptingEngine::mathFunctionDoc(const QString &name) const
{
	PyObject *mathf = PyDict_GetItemString(m_math, name.toAscii()); // borrowed
	if (!mathf) return "";
	PyObject *pydocstr = PyObject_GetAttrString(mathf, "__doc__"); // new
	QString qdocstr = PyString_AsString(pydocstr);
	Py_XDECREF(pydocstr);
	return qdocstr;
}

const QStringList PythonScriptingEngine::fileExtensions() const
{
	QStringList extensions;
	extensions << "py" << "PY";
	return extensions;
}

Q_EXPORT_PLUGIN2(scidavis_python, PythonScriptingEngine)
