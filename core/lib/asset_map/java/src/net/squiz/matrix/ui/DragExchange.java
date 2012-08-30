/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: DragExchange.java,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.ui;

import java.awt.datatransfer.*;
import net.squiz.matrix.matrixtree.MatrixTreeTransferable;
import java.awt.*;
import java.util.*;
import javax.swing.tree.*;

public class DragExchange {

	private static MatrixTreeTransferable transfer;
	private static Draggable dragSource;
	private static boolean inExchange = false;
	// cannot instantiate
	private DragExchange() {}

	public static void setTransferable(Draggable dragSource, MatrixTreeTransferable transfer) {
		if (inExchange)
			throw new IllegalStateException("There is already an exchange open by " + dragSource.getClass());
		DragExchange.transfer = transfer;
		DragExchange.dragSource = dragSource;
		inExchange = true;
	}

	public static MatrixTreeTransferable getTransferable() {
		if (!inExchange)
			throw new IllegalStateException("There is no exchange open");
		return transfer;
	}

	public static Draggable getSource() {
		if (!inExchange)
			throw new IllegalStateException("There is no exchange open");
		return dragSource;
	}

	public static TreePath[] getDragPaths() {
		if (!inExchange)
			throw new IllegalStateException("There is no exchange open");
		java.util.List paths = null;
		try {
			paths = (java.util.List) transfer.getTransferData(
				MatrixTreeTransferable.TREE_NODE_FLAVOUR);
		} catch (UnsupportedFlavorException ufe) {
		//	GUIUtilties.error();
			ufe.printStackTrace();
		} catch (java.io.IOException ioe) {
		//	GUIUtilties.error();
			ioe.printStackTrace();
		}
		return (TreePath[]) paths.toArray(new TreePath[paths.size()]);
	}

	public static Image translateImage(Draggable draggable) {
		if (!inExchange)
			throw new IllegalStateException("There is no exchange open");
		TreePath[] pathsArr = getDragPaths();

		return draggable.getDragImage(pathsArr);
	}

	public static void completeExchange() {
		if (!inExchange)
			throw new IllegalStateException("There is no exchange open");
		transfer = null;
		dragSource = null;
		inExchange = false;
	}
}
