package ij.io;
import java.awt.*;
import java.awt.image.*;
import java.io.*;
import java.net.URL;
import java.net.*;
import java.util.zip.*;
import java.util.Locale;
import javax.swing.*;
import javax.swing.filechooser.*;
import ij.*;
import ij.gui.*;
import ij.process.*;
import ij.plugin.frame.*;
import ij.text.TextWindow;
import ij.util.Java2;
import java.util.LinkedList;
import java.util.Iterator;

public class FileExtensionFilter extends javax.swing.filechooser.FileFilter
{
	private String[] _extensions = null;
	private String _description = "";
	private boolean _showExtensions = true;

	FileExtensionFilter(String description, String[] extensions) 
	{
		this._description = description;
		this._extensions = extensions;
	}

	FileExtensionFilter(String description, LinkedList extensions) 
	{
		String[] exts = new String[extensions.size()];
		int i=0;
		for (Iterator itr = extensions.iterator(); itr.hasNext(); i++) {
			exts[i] = (String)itr.next();
		}
		this._description = description;
		this._extensions = exts;
	}

	
	public boolean accept(File f) 
	{
		if (f.isDirectory()) return true;
		String extension = f.getPath();
		extension = extension.substring(extension.lastIndexOf(".")+1).toLowerCase();
		for (int i=0; i < _extensions.length; i++) {
			if (extension.equalsIgnoreCase(_extensions[i])) {
				return true;
			}
		}
		return false;
	}

	public String getDescription() 
	{
		return _description + (_showExtensions ? " (" + _getExtensionList() + ")" : "");
	}

	private String _getExtensionList() 
	{
		StringBuffer sb = new StringBuffer();
		for (int i=0; i < _extensions.length; i++) {
			sb.append("*.");
			sb.append(_extensions[i]);
			sb.append(", ");
		}
		return sb.substring(0, sb.length()-2); // remove the final comma
	}

	public String[] getExtensions() 
	{
		return _extensions;
	}

	public void setShowExtensions(boolean b) 
	{
		_showExtensions = b;
	}

}
