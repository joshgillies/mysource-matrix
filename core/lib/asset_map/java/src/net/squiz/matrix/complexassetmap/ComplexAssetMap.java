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
* $Id: ComplexAssetMap.java,v 1.4 2004/11/02 02:51:46 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.complexassetmap;

import net.squiz.matrix.assetmap.*;

import javax.swing.*;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.UIManager;
import netscape.javascript.*;

//import com.sun.java.swing.plaf.windows;


public class ComplexAssetMap extends AssetMap /*implements KeyListener*/
{

	/** The tabbed pane for the 3 sections of the asset map */
	private JTabbedPane tp;

	/** The name main frame in the browser */
	public static final String SQ_MAIN_FRAME = "sq_main";

	/** The font used on tabs*/
	public static final Font TAB_FONT = new Font("Tab Font", Font.PLAIN, 10);

	public static final int POLLING_DELAY = 2000;

	/**
	 * Constructor
	 */
	public ComplexAssetMap() {

		// you must set the following properties before
		// creating the tabbed pane object
		UIManager.put("TabbedPane.selected", AssetMapMenuPanel.BG_COLOUR);
		UIManager.put("TabbedPane.background", new Color(0x725B7D));
		UIManager.put("TabbedPane.foreground", Color.white);

		tp = new JTabbedPane();
	}

	/**
	 * Initialises the tree.
	 */
	public void initTree() {
		tree = new ComplexAssetTree(new DefaultTreeModel(
				AssetManager.INSTANCE.getRootNode()));
	//	tree.addKeyListener(this);
		tree.initialise();

		ActionListener taskPerformer = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {

				if (window == null)
					JSObject.getWindow(ComplexAssetMap.this);

				String assetids = (String) window.getMember("SQ_REFRESH_ASSETIDS");
				if (!assetids.equals("")) {
					AssetManager.INSTANCE.assetsReloaded(assetids);
					window.eval("SQ_REFRESH_ASSETIDS = '';");
				}
			}
		};

		Timer t = new Timer(POLLING_DELAY, taskPerformer);
		t.start();
	}

	/**
	 * Returns the Asset Tree
	 *
	 * @return the Asset Tree
	 */
	public JTree getTree() {
		return this.tree;
	}

	/**
	 * Initialises the applet
	 */
	public void start() {

		Icon mySpaceIcon = MatrixToolkit.getAssetMapIcon("myspace.png");
		Icon treeIcon = MatrixToolkit.getAssetMapIcon("tree.png");

		JPanel treePanel = new JPanel(new BorderLayout());
		treePanel.add(new AssetMapMenuPanel((ComplexAssetTree) tree), BorderLayout.NORTH);
		treePanel.add(new JScrollPane(tree));

		StatusKey key = new StatusKey();
		key.setSize(500, 500);

		JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, treePanel, key);
		splitPane.setResizeWeight(1.0);
		splitPane.setDividerLocation(Integer.MAX_VALUE);
		splitPane.setOneTouchExpandable(true);

		tp.setFont(TAB_FONT);
		tp.addTab("Asset Map", treeIcon, splitPane);

		String workspaceId = AssetManager.INSTANCE.getWorkspaceId();

		if (!workspaceId.equals("0")) {

			String inboxURL = MySource.INSTANCE.getBaseURL() +
				AssetMap.getApplet().getParameter("BACKEND_SUFFIX") +
				MatrixToolkit.rawUrlDecode(getParameter("INBOX_URL"));

			String detailsURL = MySource.INSTANCE.getBaseURL() +
				AssetMap.getApplet().getParameter("BACKEND_SUFFIX") +
				MatrixToolkit.rawUrlDecode(getParameter("DETAILS_URL"));

			int newMessages = Integer.parseInt(getParameter("NEW_MSGS"));

			tp.addTab("My Space", mySpaceIcon, new MySpace(
					inboxURL,
					detailsURL,
					workspaceId,
					newMessages));

		}

		getContentPane().setLayout(new BorderLayout());
		getContentPane().add(tp);
	}

	/*private int keysPressed = 0;
	private String trigger = "invaders";
	public void keyPressed(KeyEvent e) {
		if (e.getKeyChar() == trigger.charAt(keysPressed)) {
			keysPressed++;
			if (keysPressed == trigger.length()) {
				tp.setTitleAt(1, "My Space Invaders");
			}
		} else {
			keysPressed = 0;
		}
	}

	public void keyReleased(KeyEvent e) {}
	public void keyTyped(KeyEvent e) {}*/
}
