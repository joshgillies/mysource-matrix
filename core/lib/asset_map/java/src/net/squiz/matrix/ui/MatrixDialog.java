/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: MatrixDialog.java,v 1.4 2006/12/05 05:26:37 bcaldwell Exp $
*
*/

package net.squiz.matrix.ui;

import net.squiz.matrix.plaf.MatrixLookAndFeel;
import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.core.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;
import java.awt.*;
import java.util.*;

/**
 *
 */
public class MatrixDialog extends JDialog {

	private int x= -1;
	private int y= -1;
	private static HashMap dialogs = new HashMap();
	public Cursor HAND_CURSOR = new Cursor(Cursor.HAND_CURSOR);
	public Cursor DEFAULT_CURSOR = new Cursor(Cursor.DEFAULT_CURSOR);

	public MatrixDialog() {
		setUndecorated(true);
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


	Point prevLoc;

	public Point getPrevLoc() {
		return prevLoc;
	}

	private void setPrevLoc(Point prevLoc) {
		this.prevLoc = prevLoc;
	}


	/**
	* Set the location of the dialog to the center of the tree.
	* Make sure you call this after calling pack()
	*/
	public void centerDialogOnTree(Point locationOnScreen, Dimension treeDimension, Point prevLoc) {
		setPrevLoc(prevLoc);

		int locX = (int)locationOnScreen.getX()+(int)((treeDimension.getWidth()/2)-(getWidth()/2));
		if (locX < 0) {
			locX = 5;
		}

		setLocation(locX,(int)(locationOnScreen.getY()+(treeDimension.getHeight()/3)), false);
	}

	public JPanel getTopPanel(String dialogTitle) {
		JPanel topPanel = new JPanel();
		topPanel.setBorder(new EmptyBorder(0, 0, 0, 0));
		topPanel.setBackground(MatrixLookAndFeel.PANEL_COLOUR);

		JLabel title = new JLabel(dialogTitle);
		title.setFont(MatrixTreeBus.getActiveTree().getFontInUse());
		title.setHorizontalTextPosition(SwingConstants.CENTER);
		title.setOpaque(false);
		title.setForeground(Color.black);

		topPanel.add(title);
		enableDrag(topPanel);

		return topPanel;
	}

	public void closeOnClick(final JLabel component, final String img_prefix) {
		component.addMouseListener(new MouseAdapter(){
			public void mouseClicked(MouseEvent e){
				setCursor(DEFAULT_CURSOR);
				dispose();
			}

			public void mouseExited(MouseEvent e) {
				component.setIcon(GUIUtilities.getAssetMapIcon(img_prefix + ".png"));
				setCursor(DEFAULT_CURSOR);
			}

			public void mouseEntered(MouseEvent e) {
				setCursor(HAND_CURSOR);
				component.setIcon(GUIUtilities.getAssetMapIcon(img_prefix + "_on.png"));
			}
		});
	}

	public void enableDrag(final JComponent component) {
		component.addMouseListener(new MouseAdapter(){
			public void mouseReleased(MouseEvent e){
				x = e.getX();
				y = e.getY();
			}
			public void mousePressed(MouseEvent e){
				x = e.getX();
				y = e.getY();
			}
		});

		component.addMouseMotionListener(new MouseMotionAdapter(){
			public void mouseDragged(MouseEvent e){
				int posX = getLocationOnScreen().x+(e.getX()-x);
				int posY = getLocationOnScreen().y+(e.getY()-y);
				setLocation(posX, posY, true);
				setPrevLoc(new Point(posX, posY));
			}
		});
	}



	public void setLocation(int x, int y, boolean force) {
		if (force || (getPrevLoc() == null)) {
			super.setLocation(x, y);
		} else {
			x = (int)getPrevLoc().getX();
			y = (int)getPrevLoc().getY();
			super.setLocation(x, y);
		}
		setPrevLoc(new Point(x, y));
	}
}
