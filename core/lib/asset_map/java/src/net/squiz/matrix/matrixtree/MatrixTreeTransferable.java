
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
