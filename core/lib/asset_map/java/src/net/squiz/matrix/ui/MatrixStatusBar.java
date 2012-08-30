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
* $Id: MatrixStatusBar.java,v 1.6 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.ui;

import javax.swing.*;
import java.awt.*;
import java.util.*;
import java.awt.event.*;
import net.squiz.matrix.core.*;
import javax.swing.border.*;


/**
 * MatrixStatusBar provides a browser like status bar to inform the user
 * of the current operations that the AssetMap is performing.
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixStatusBar {

	private static java.util.List elements = new ArrayList();
	private static MatrixStatusBarElement[] elementsArr;

	/* the text that was last set globally */
	private static String statusText = "";

	// cannot instantiate
	private MatrixStatusBar() {}

	/**
	 * Returns a new JPanel that will be updated with matrix status information
	 * @return JPanel the status bar
	 */
	public static MatrixStatusBarElement createStatusBar() {
		MatrixStatusBarElement element = new MatrixStatusBarElement(statusText);
		synchronized(elements) {
			elements.add(element);
		}
		return element;
	}

	/**
	 * Returns the elements from the element collection in array form. The
	 * elements are cached, and the cache is updated whenever an element is added
	 * or removed from the element collection. You should use this method in favour
	 * of an iterator.
	 * @return the elements in array form
	 */
	private static MatrixStatusBarElement[] getElements() {
		synchronized(elements) {
			if (elementsArr == null) {
				elementsArr = (MatrixStatusBarElement[]) elements.toArray(
					new MatrixStatusBarElement[elements.size()]);
			}
			return elementsArr;
		}
	}

	/**
	 * Sets the global status for the status bars.
	 * @param status the status to set globally
	 * @see MatrixStatusBarElement.setStatus(String)
	 */
	public static void setStatus(String status) {
		statusText = status;
		MatrixStatusBarElement[] elements = getElements();
		for (int i = 0; i < elements.length; i++)
			elements[i].setStatus(status);
	}

	/**
	 * clears the status of all status bars
	 * @see MatrixStatusBarElement.clearStatus()
	 */
	public static void clearStatus() {
		setStatus("");
	}

	/**
	 * Returns the status that was set in the last global <code>setStatus</code>
	 * method invocation.
	 * @return the last global status that was set
	 * @see MatrixStatusBarElement.getStatus()
	 */
	public static String getStatus() {
		return statusText;
	}

	/**
	 * Sets the status and the clears it after waiting for the specified time
	 * in milliseconds.
	 * @param status the status to set globally
	 * @param time the time to wait in milliseconds before clearing the status
	 * globally
	 * @see MatrixStatusBarElement.setStatusAndClear(String, int)
	 */
	public static void setStatusAndClear(String status, int time) {
		// dont call MatrixStatusBarElement.setStatusAndClear as it will
		// create an action listener and timer for all status bars and cause
		// undesirable results.
		ActionListener listener = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				setStatus("");
			}
		};
		setStatus(status);
		javax.swing.Timer t = new javax.swing.Timer(time, listener);
		t.setRepeats(false);
		t.start();
	}

	/**
	 * An individual status bar element. The status bar consists of a spinner
	 * to indicate that an operation is progress, and a JLabel to display status
	 * information.
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	private static class MatrixStatusBarElement extends JPanel
		implements MatrixConstants, MouseListener {

		private JLabel label;
		private Spinner spinner;
		private javax.swing.Timer clearTimer;

		// only MatrixStatusBar should be able to create individual elements
		private MatrixStatusBarElement(String status) {
			setLayout(new FlowLayout(FlowLayout.LEFT));

			spinner  = new Spinner();
			label    = new JLabel(statusText);

			label.setFont(PLAIN_FONT_10);

			add(spinner);
			add(label);

			setBackground(UIManager.getColor("StatusBar.background"));

			// create an action and a timer to clear the status
			// which will also stop the spinner
			ActionListener listener = new ActionListener() {
				public void actionPerformed(ActionEvent evt) {
					setStatus("");
				}
			};
			clearTimer = new javax.swing.Timer(1000, listener);
			clearTimer.setRepeats(false);
			addMouseListener(this);
		}

		/**
		 * Sets the status of this individual StatusBarElement
		 * @param status the status to set for this element
		 * @see MatrixStatusBar.setStatus(String)
		 */
		private void setStatus(String status) {
			if (status.equals("") && spinner.isStarted())
				spinner.stop();
			else if ((!status.equals("")) && !spinner.isStarted())
				spinner.start();
			label.setText(status);
		}

		/**
		 * Clears the status for this individual status bar element
		 * @see MatrixStatusBar.clearStatus()
		 */
		private void clearStatus() {
			setStatus("");
		}

		/**
		 * Sets the status and the clears it after waiting for the specified time
		 * in milliseconds.
		 * @param status the status to set globally
		 * @param time the time to wait in milliseconds before clearing the status
		 * globally
		 * @see MatrixStatusBar.setStatusAndClear(String, int)
		 */
		private void setStatusAndClear(String status, int time) {
			setStatus(status);
			clearTimer.setDelay(time);
			clearTimer.start();
		}

		/**
		 * Returns the status of this individual status bar element
		 * @return the status of this individual status bar element
		 * @see MatrixStatusBar.getStatus()
		 */
		private String getStatus() {
			return label.getText();
		}

		public void mouseEntered(MouseEvent evt) {}
		public void mouseExited(MouseEvent evt) {}
		public void mousePressed(MouseEvent evt) {}
		public void mouseReleased(MouseEvent evt) {}

		public void mouseClicked(MouseEvent evt) {
			if (evt.getClickCount() != 2)
				return;
			/*Runtime rt = Runtime.getRuntime();
			long preFree = rt.freeMemory();
			rt.gc();
			long postFree = rt.freeMemory();
			long freed = (postFree - preFree) / 1000;
			GUIUtilities.error(freed + "Kb Released", "Memory Released");*/
			net.squiz.matrix.debug.Log.openLogs();
		}
	}
}
