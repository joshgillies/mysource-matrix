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
* $Id: AssetTree.java,v 1.3 2003/11/26 00:51:13 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

package squiz.matrix;

import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import javax.swing.event.TreeWillExpandListener;
import javax.swing.event.TreeExpansionEvent;

import javax.swing.JTree;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JOptionPane;
import javax.swing.ToolTipManager;
import javax.swing.JPopupMenu;
import javax.swing.JMenuItem;

import javax.swing.Action;
import javax.swing.AbstractAction;

import java.awt.GridLayout;

import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.ExpandVetoException;

import java.util.Iterator;

import java.io.IOException;

import org.w3c.dom.Element;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;


/**
 * The Asset Tree panel. Displays an asset's tree, and provides an interface 
 * for users to add and manipulate assets.
 * 
 * <p><code>$Id: AssetTree.java,v 1.3 2003/11/26 00:51:13 gsherwood Exp $</code></p>
 *
 * @author				Dominic Wong <dwong@squiz.net>
 * @see AssetTreeModel	squiz.matrix.AssetTreeModel
 * @see MySource		squiz.matrix.MySource
 */
public class AssetTree extends JPanel implements TreeWillExpandListener, MouseListener
{
	/** the JTree widget */
	private JTree tree;
	/** the asset tree model */
	private AssetTreeModel model;

	public AssetTree() {
		model = new AssetTreeModel();
		tree = new JTree();
	}//end constructor


	/** 
	 * Sets up the widget, but also initialises the asset factory, asset type 
	 * factory and the mysource interface.
	 *
	 * <p><b>Note</b>: this should probably be moved to class MySource
	 * 
	 * @throws IOException	if an IOException occurs while doing the XML
	 *						request.
	 */
	public void init() throws IOException {
		AssetFactory af = AssetFactory.getInstance();
		MySource mysource = MySource.getInstance();
		AssetTypeFactory atf = AssetTypeFactory.getInstance();

		Document response = mysource.doRequest("<command action=\"initialise\" />");
		NodeList children = response.getDocumentElement().getChildNodes();

		for (int i = 0; i < children.getLength(); ++i) {
			if (!(children.item(i) instanceof Element))
				continue;
			
			Element childElement = (Element)children.item(i);
			if (childElement.getTagName().equals("asset_types")) {
				try {
					atf.processAssetTypesElement(childElement);
				} catch (AssetTypeNotFoundException atnfe) {
					throw new IOException ("Asset type not found : " + atnfe.getMessage());
				}
				continue;
			} else if (childElement.getTagName().equals("assets")) {
				af.processAssetsElement(childElement);
				continue;
			}
		}

		ToolTipManager.sharedInstance().registerComponent(tree);

		tree.setRootVisible(false);
		tree.setShowsRootHandles(true);

		tree.addTreeWillExpandListener(this);
		tree.addMouseListener(this);

		tree.setModel(model);
		tree.setCellRenderer(new AssetCellRenderer(model));
		tree.expandPath(new TreePath(model.getRoot()));

		setLayout(new GridLayout(1, 1));
		add(new JScrollPane(tree));

	}//end init()

	/* TreeExpansionListener methods */

	/** 
	 * Expands an asset, possibly retrieving any assets that are children of 
	 * this asset that have not been loaded yet.
	 * 
	 * @param e						the <code>TreeExpansionEvent</code>
	 * @throws ExpandVetoException	if there was an exception while retrieving 
	 *								assets
	 */
	public void treeWillExpand(TreeExpansionEvent e) throws ExpandVetoException {
		
		TreePath path = e.getPath();
		Asset asset = model.getAssetFromNode(path.getLastPathComponent());
		AssetFactory af = AssetFactory.getInstance();

		try {
			af.updateAsset(asset);
		} catch(IOException io) {
			String msg = "Error while attempting to update asset " + asset + " : \n" + io.getMessage();
			String title = "Error";
			int messageType = JOptionPane.ERROR_MESSAGE;
			JOptionPane.showMessageDialog(this, msg, title, messageType);
			throw new ExpandVetoException(e);
		}
	}//end treeWillExpand()
	

	public void treeWillCollapse(TreeExpansionEvent e) { }//end treeWillCollapse()

	/* end TreeExpansionListener methods */

	/* MouseListener methods */
	public void mouseClicked(MouseEvent e) {}//end mouseClicked()
	public void mouseEntered(MouseEvent e) {}//end mouseEntered()
	public void mouseExited(MouseEvent e) {}//end mouseExited()
	public void mousePressed(MouseEvent e) {}//end mousePressed()


	/** 
	 * Shows the asset action menu when the right mouse button is released on 
	 * the tree for the currently selected path. 
	 * 
	 * @param e the mouse event
	 * @see MouseListener java.awt.event.MouseListener
	 */
	public void mouseReleased(MouseEvent e) {
		if (e.getButton() != MouseEvent.BUTTON3)
			return;
		TreePath clickPath = tree.getSelectionPath();
		if (clickPath != null) {
			Asset asset = model.getAssetFromNode(clickPath.getLastPathComponent());
			showAssetMenu(asset, e.getX(), e.getY());
			tree.repaint();
		}

	}//end mouseReleased()

	/* end MouseListener methods */


	/**
	 * Shows the asset action menu for an asset at the specified coordinates.
	 * 
	 * @param asset		The asset for which to print out
	 * @param x			The x-coordinate in the <code>AssetTree</code>'s 
	 *					coordinate-space
	 * @param y			The y-coordinate in the <code>AssetTree</code>'s 
	 *					coordinate-space
	 */
	private void showAssetMenu(Asset asset, int x, int y) {
		JPopupMenu menu = new JPopupMenu(asset.getName());
		AssetType type = asset.getType();
		Iterator screens = type.getScreens();
		
		while(screens.hasNext()) {
			AssetTypeScreen screen = (AssetTypeScreen)screens.next();
			Action action = new AbstractAction() {
				public void actionPerformed(ActionEvent e) {
					System.err.println(getValue("code name"));
				}
			};

			action.putValue(Action.NAME, screen.screenName);
			action.putValue("code name", screen.codeName);
			JMenuItem item = new JMenuItem(action);
			menu.add(item);
		}

		menu.show(this, x, y);

	}//end showAssetMenu()


	/**
	 * Testing method.
	 */
	public static void main (String[] args) throws IOException {
		JFrame fudge = new JFrame("Asset Tree Test");
		AssetTree at = new AssetTree();
		fudge.setSize(400, 400);
		fudge.show();
		fudge.getContentPane().add(at);

		fudge.addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				System.exit(0);
			}
		});

		at.init();
		fudge.pack();
	}//end main()

}//end class