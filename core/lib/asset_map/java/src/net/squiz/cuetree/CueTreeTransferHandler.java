
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

		System.out.println("right here mother ficker");
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
