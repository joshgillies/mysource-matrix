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
* $Id: AssetTree.java,v 1.1 2004/01/13 00:44:33 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.awt.event.*;
import java.awt.*;
import org.w3c.dom.*;

import java.io.IOException;

import javax.swing.plaf.basic.BasicTreeUI;
import com.sun.java.swing.plaf.windows.*;

/**
* The base (Simple) Asset Tree. This tree is used for the asset finder to select assets
* Assets cannot be draged and dropped
*
* @author Marc McIntyre <mmcintyre@squiz.net>
* @see AssetTreeModelnet.squiz.matrix.assetmap.AssetTreeModel
* @see MySource net.squiz.matrix.assetmap.MySource
*/
public class AssetTree extends JTree implements TreeWillExpandListener, MouseListener, AssetFinderListener {
	

	/**
	* the asset tree model
	*/
	protected AssetTreeModel model = null;

	/**
	* Boolean for the state of the current mode of the tree
	*/
	protected boolean isInAssetFinderMode = false;

	/**
	* if TRUE we are in search mode
	*/
	private boolean isInSearchMode = false;

	/**
	* The frame to show the search options
	*/
	private JFrame searchFrame = null;


	/**
	* Constructor
	*/
	public AssetTree() {
		super();

		model = new AssetTreeModel();
		registerKeyboardActions();
		
		UIManager.put("Tree.expandedIcon", new WindowsTreeUI.ExpandedIcon());
		UIManager.put("Tree.collapsedIcon", new WindowsTreeUI.CollapsedIcon());
		updateUI();

	}//end constructor


	/**
	* Returns <code>True</code> if the tree is currently in asset finder mode
	*
	* @return the current state of the tree
	*/
	public final boolean isInAssetFinderMode() {
		return isInAssetFinderMode;
	
	}//end isInAssetFinderMode()


	/**
	* Sets the asset finder mode
	*
	* @param mode the mode to set
	*/
	public final void setAssetFinderMode(boolean mode) {
		isInAssetFinderMode = mode;
		if (!mode) {
			model.setAllNodesGrey();
			repaint();
		}

	}//end setAssetFinderMode()


	/**
	* Initialises the tree
	*
	* This includes: <br />
	*
	* <ul>
	*   <li>Requesting the assets fron <code>asset_map.inc</code></li>
	*   <li>Registering listeners</li>
	*   <li>Creating factory instances</li>
	* </ul>
	*/
	public void init() throws IOException {

		// create some factory instances
		AssetFactory af = AssetFactory.getInstance();
		MySource mysource = MySource.getInstance();
		AssetTypeFactory atf = AssetTypeFactory.getInstance();

		// do a request to the mysource object to get a list of the nodes
		Document response = mysource.doRequest("<command action=\"initialise\" />");
		NodeList children = response.getDocumentElement().getChildNodes();

		// foreach node, sort out which is the asset types and which are the actual assets, and process them
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
		}//end for
		
		ToolTipManager.sharedInstance().registerComponent(this);

		// we don't want to see the root folder
		setRootVisible(false);
		setShowsRootHandles(true);

		addTreeWillExpandListener(this);
		addMouseListener(this);

		this.setModel(model);
		setCellRenderer(new AssetCellRenderer(model));
		expandPath(new TreePath(model.getRoot()));

		// register that we want to listen for asset_finder events
		JsEventManager.sharedInstance().addJsListener("asset_finder", this);

	}//end init()


	/** 
	 * Expands an asset, possibly retrieving any assets that are children of 
	 * this asset that have not been loaded yet.
	 * 
	 * @param e the <code>TreeExpansionEvent</code>
	 * @throws ExpandVetoException	if there was an exception while retrieving assets
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
	

	/**
	* Returns the selected node, which is a <code>AssetLink</code>
	* 
	* @return the <code>AssetLink</code> node
	* @see  <code>net.squiz.matrix.assetmap.AssetTreeModel.getAssetFromNode()</code>
	*/
	public AssetLink getSelectedNode() {
		TreePath clickPath = this.getSelectionPath();
		
		AssetLink node = null;
		if (clickPath != null) {
			node = (AssetLink) clickPath.getLastPathComponent();
		}
		return node;

	}//end getSelectedNode()


	/**
	* Abstract Method
	*
	* @param e the tree expansion event
	*/
	public void treeWillCollapse(TreeExpansionEvent e) { }//end treeWillCollapse()


	/**
	* Abstract Method
	*
	* @param e the mouse event
	*/
	public void mouseClicked(MouseEvent e) {}//end mouseClicked()
	
	
	/**
	* Abstract Method
	*
	* @param e the mouse event
	*/
	public void mouseEntered(MouseEvent e) {}//end mouseEntered()


	/**
	* Abstract Method
	*
	* @param e the mouse event
	*/
	public void mouseExited(MouseEvent e) {}//end mouseExited()
	
	
	/**
	* Catches a right mouse click and and selects that node (which the JTree does not do by default)
	*
	* @param e the mouse event
	*/
	public void mousePressed(MouseEvent e) {
		// we want to select the path whenever the right mouse button is clicked...
		// we need to select the node before showing the menu, 
		// be it the "use me" menu of the screens menu
		// we don't need to worry about left clicks because the JTree will take care of this for us
		if (e.getButton() != MouseEvent.BUTTON3) 
			return;
		TreePath clickPath = getPathForLocation(e.getX(), e.getY());
		if (clickPath != null) {
			setSelectionPath(clickPath);
		}

	}//end mousePressed()

	
	/**
	* Gets called when the javascript initiates the asset finder
	* 
	* @param e the javascript event 
	*
	* @see <code>JsEventManager</code>
	* @see <code>AssetFinderListener</code>
	*/
	public void assetFinderStarted(JsEvent e) {
		
		model.clearRestrictedAssetTypes();
		// we are now in asset finder mode
		setAssetFinderMode(true);

		String assetTypes = (String) e.getParams().get("type_codes");

		Set typeCodes = new HashSet();
		if (assetTypes != null) {
			StringTokenizer st = new StringTokenizer(assetTypes, "|");
			while (st.hasMoreTokens()) {
				typeCodes.add((String) st.nextToken());
			}
		}

		// if there are no asset types, then just return
		if (!(typeCodes.isEmpty())) {
			// tell the model what the restricted types are
			model.setRestrictedAssetTypes(typeCodes);
		}
		// repaint me
		repaint();

	}//end assetFinderStarted()


	/**
	* Gets called when the asset finder is canceled
	*
	* @param e the javascript event
	*
	* @see <code>JsEventManager</code>
	* @see <code>AssetFinderListener</code>
	*/
	public void assetFinderStopped(JsEvent e) {

		setAssetFinderMode(false);

	}//end assetFinderStopped()


	/**
	* Shows the asset action menu when the right mouse button is released on 
	* the tree for the currently selected path. 
	* 
	* @param e the mouse event
	* @see MouseListener java.awt.event.MouseListener
	*/
	public void mouseReleased(MouseEvent e) {
		if (!isInAssetFinderMode())
			return;
		if (e.getButton() != MouseEvent.BUTTON3)
			return;
		
		AssetLink node = getSelectedNode();
		if (node != null && model.isNodeEnabled(node)) {
			Asset asset = model.getAssetFromNode(node);
			showUseMeMenu(asset, e.getX(), e.getY());
			this.repaint();
		}

	}//end mouseReleased()


	/**
	* Registers keyboard actions that can be caught to perform actions
	*
	* Some actions might be deleting, opening the find menu
	*/
	public void registerKeyboardActions() {
	
		KeyStroke ks = KeyStroke.getKeyStroke(KeyEvent.VK_F, Event.CTRL_MASK);
		ActionListener actionListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				showFindPopup();
			}
		};

		registerKeyboardAction(actionListener, "Find", ks, JComponent.WHEN_FOCUSED);

	}//end registerKeyboardActions()


	/**
	* Shows the Find Frame to enter search options
	*
	* @see #registerKeyboardActions
	*/
	public void showFindPopup() {
	
		// if there is already a popup open, don't show another one, just
		// get focus on the current one
		if (isInSearchMode) {
			if (searchFrame != null) {
				searchFrame.toFront();
			}
			return;
		}

		// if we weren't before, we are now!
		isInSearchMode = true;
	
		Action okAction = new AbstractAction() {
			public void actionPerformed(ActionEvent e) {
				searchFrame.dispose();
				isInSearchMode = false;
			}
		};
		okAction.putValue(Action.NAME, "Search");

		Action cancelAction = new AbstractAction() {
			public void actionPerformed(ActionEvent e) {
				searchFrame.dispose();
				isInSearchMode = false;
			}
		};
	
		cancelAction.putValue(Action.NAME, "Cancel");

		Dimension buttonSize = new Dimension(190, 100);

		JTextField textField = new JTextField(20);
		JButton okBtn = new JButton(okAction);
		JButton cancelBtn = new JButton(cancelAction);
		JCheckBox isAssetId = new JCheckBox("Search on Asset Id");

		okBtn.setPreferredSize(buttonSize);
		cancelBtn.setPreferredSize(buttonSize);

		Dimension offset = new Dimension(19, 20);

		// create a button panel and add the buttons to it
		JPanel btnPanel = new JPanel();
		btnPanel.setLayout(new BoxLayout(btnPanel, BoxLayout.X_AXIS));
		btnPanel.add(new javax.swing.Box.Filler(offset, offset, offset));
		btnPanel.add(okBtn);
		btnPanel.add(cancelBtn);

		// get the user's screen size
		Dimension d = Toolkit.getDefaultToolkit().getScreenSize();

		// create it if its null
		if (searchFrame == null) {
			searchFrame = new JFrame("Find an Asset");
		}
	
		// add the components to the frame
		searchFrame.getContentPane().add(btnPanel, BorderLayout.SOUTH);
		searchFrame.getContentPane().add(textField, BorderLayout.NORTH);
		
		int frameWidth  = 400;
		int frameHeight = 150;

		// get the co-ords of where to put this frame so it is in the center of the screen
		int frameX = (int) (d.getWidth() - frameWidth) / 2;
		int frameY = (int) (d.getHeight() - frameHeight) / 2;

		// the frame does not need to be resized, OK!
		searchFrame.setResizable(false);
		//set the frame size
		searchFrame.setSize(frameWidth, frameHeight);
		//display the popup in the middle of the screen
		searchFrame.setLocation(frameX, frameY);
		
		searchFrame.setVisible(true);

	}//end showFindPopup()


	/**
	* Shows the use me menu for the asset finder
	* 
	* @param asset		The asset for which to print out
	* @param x			The x-coordinate in the <code>AssetTree</code>'s coordinate-space
	* @param y			The y-coordinate in the <code>AssetTree</code>'s coordinate-space
	*/
	private void showUseMeMenu(Asset asset, int x, int y) {
		JPopupMenu menu = new JPopupMenu(asset.getName());
		AssetType type = asset.getType();
		
		Action action = new AbstractAction() {
			public void actionPerformed(ActionEvent e) {

				// pack an array with vars to be passed back to the javascript
				String [] args = new String[3];
				args[0] = (String) getValue("assetid");
				args[1] = (String) getValue("label");
				args[2] = (String) getValue("url");
				// call the javascript method with the event type
				JsEventManager.getInstance().javaToJsCall("asset_finder", args);
				// reset the state of the asset finder
				setAssetFinderMode(false);
			}
		};
		action.putValue(Action.NAME, "Use Me");
		action.putValue("assetid", Integer.toString(asset.getId()));
		action.putValue("url", asset.getUrl());
		action.putValue("label", asset.getName());

		JMenuItem item = new JMenuItem(action);
		menu.add(item);
		menu.show(this, x, y);

	}//end ShowAssetMenu()


}//end class