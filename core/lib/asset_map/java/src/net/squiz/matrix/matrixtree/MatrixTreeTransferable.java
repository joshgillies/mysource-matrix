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
* $Id: MatrixTreeTransferable.java,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.matrixtree;

import java.awt.datatransfer.*;
import java.util.*;
import java.io.*;
import javax.swing.tree.*;

public class MatrixTreeTransferable implements Transferable, Serializable {

	public static final DataFlavor TREE_NODE_FLAVOUR
		= new DataFlavor(MatrixTreeNode.class, "Matrix Tree Node");

	static DataFlavor flavors[] = { TREE_NODE_FLAVOUR };

	private final List paths;

	public MatrixTreeTransferable(TreePath[] paths) {
		this.paths = Arrays.asList(paths);
	}

	public Object getTransferData(DataFlavor flavor)
		throws UnsupportedFlavorException, IOException {
			if (flavor.equals(TREE_NODE_FLAVOUR))
				return paths;
			else
				throw new UnsupportedFlavorException(flavor);
	}

	public DataFlavor[] getTransferDataFlavors() {
		return flavors;
	}

	public boolean isDataFlavorSupported(DataFlavor flavor) {
		return flavor.equals(TREE_NODE_FLAVOUR);
	}
}
