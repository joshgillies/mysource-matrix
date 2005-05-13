/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: CueTreeTransferHandler.java,v 1.2 2005/05/13 02:14:58 ndvries Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.cuetree;

import javax.swing.*;
import java.awt.event.*;
import java.awt.datatransfer.*;
import java.util.*;

public class CueTreeTransferHandler extends TransferHandler {

	public CueTreeTransferHandler() {
		System.out.println("DragAndDropTransferHandler created");
	}

	public boolean canImport(JComponent comp, DataFlavor[] transferFlavors) {

		for (int i = 0; i < transferFlavors.length; i++) {
			if (transferFlavors[i].equals(DataFlavor.javaFileListFlavor)) {
				System.out.println("was a file");
				return true;
			}
		}
		System.out.println("There were no files in the clipboard");
		return false;
	}

	public boolean importData(JComponent c, Transferable t) {

		System.out.println("Start of importData");

		if (t.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
			List files = null;
			try {
				files = (List) t.getTransferData(DataFlavor.javaFileListFlavor);
			} catch (Exception e) {
				e.printStackTrace();
			}
			System.out.println(files);
		} else {
			System.out.println("Was not a file");
		}

		System.out.println("importing data");
		return true;
	}
}
