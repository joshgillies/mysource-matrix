
package net.squiz.matrix.ui;

import javax.swing.*;
import java.util.*;

/**
 * 
 */
public class MatrixDialog extends JDialog {
	
	private static HashMap dialogs = new HashMap();

	public MatrixDialog() {
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
	}
	
	/**
	 * Returns the MatrixDialog for the given class or null if one
	 * does not yet exist in the store
	 * @param cls the class of wanted MatrixDialog
	 * @return the MatrixDialog with the given class
	 */
	public static MatrixDialog getDialog(Class cls) {
		return (MatrixDialog) dialogs.get(cls);
	}
	
	/**
	 * Puts a MatrixDialog into the store into the dialog store
	 * @param dialog the dialog to add to the store
	 */
	public static void putDialog(MatrixDialog dialog) {
		dialogs.put(dialog.getClass(), dialog);
	}
	
	/**
	 * Returns TRUE if a MatrixDialog exists in the store with the given class
	 * @return TRUE if the MatrixDialog exists with the given class
	 */
	public static boolean hasDialog(Class cls) {
		return dialogs.containsKey(cls);
	}
	
	/**
	 * Disposes the MatrixDialog and removes it from the store 
	 */
	public void dispose() {
		dialogs.remove(getClass());
		super.dispose();
	}
}
