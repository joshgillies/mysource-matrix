
package net.squiz.matrix.complexassetmap;

import javax.swing.*;
import javax.swing.tree.*;
import java.awt.*;
import net.squiz.matrix.assetmap.*;

import java.util.*;
import java.awt.event.*;
import java.net.*;
import javax.swing.border.*;

/**
 * The area where various sections useful to the user and displayed
 * 
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
class MySpace extends JPanel {

	/** the inbox url */
	private String inboxURL;
	
	/** the details url */
	private String detailsURL;
	
	/** the workspace's asset id */
	private String workspaceId;
	
	/** the number of new messages */
	private int newMessages;
	
	/** The background colour used in the MySpace Area */
	public static final Color BG_COLOUR = Color.white;
	
	/**The colour of buttons in the MySpace area */
	public static final Color BUTTON_COLOUR = new Color(0xFAFAFA);
	
	public MySpace(String inboxURL, 
			String detailsURL, 
			String workspaceId,
			int newMessages) {
		
		this.inboxURL    = inboxURL;
		this.detailsURL  = detailsURL;
		this.workspaceId = workspaceId;
		this.newMessages = newMessages;
		
		setBackground(BG_COLOUR);
		setLayout(new BorderLayout());
		add(createEntryPanel(), BorderLayout.NORTH);
	//	add(createWorkspaceTree());
	}
	
	/**
	 * Displays the sections of the MySpace Area
	 */
	private JPanel createEntryPanel() {
	
		JPanel entryPanel = new JPanel();
		entryPanel.setLayout(new BoxLayout(entryPanel, BoxLayout.Y_AXIS));
		entryPanel.setBackground(BG_COLOUR);
		
		String textColour = "#666666";	
		
		String inboxStr = "<font color=\"" + textColour + "\">Inbox</font><br>" +
				"<font color=\""+ textColour + "\" size=\"2\">" +
				"You have <b><font color=\"#1A9CF0\">[" + 
				newMessages + "]</font></b> new message(s)</font>";
		
		String detailsStr = "<font color=\"" + textColour 
				+ "\">My Details</font>";
		
		ActionListener listener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (e.getActionCommand().equals("inbox")) {
					try {
						AssetMap.getUrl(new URL(inboxURL));
					} catch (MalformedURLException mue) {
						mue.printStackTrace();
					}
				} else if (e.getActionCommand().equals("details")) {
					try {
						AssetMap.getUrl(new URL(detailsURL));
					} catch (MalformedURLException mue) {
						mue.printStackTrace();
					}
				}
			}
		};
		
		Dimension gap = new Dimension(2, 2);
		Box.Filler filler = new Box.Filler(gap, gap, gap);
		
		JButton inboxButton = createButton(inboxStr, "inbox_icon.png", listener);
		inboxButton.setActionCommand("inbox");
		entryPanel.add(inboxButton);
		
		entryPanel.add(filler);
		
		JButton detailsButton = createButton(detailsStr, "mydetails_icon.png", listener);
		detailsButton.setActionCommand("details");
		entryPanel.add(detailsButton);
		
		Dimension gap2 = new Dimension(1, 20);
		Box.Filler filler2 = new Box.Filler(gap2, gap2, gap2);
		
		entryPanel.add(filler2);
		
		JLabel workspaceLabel = 
			new JLabel(MatrixToolkit.getAssetMapIcon("workspace_icon.png"));
		
		workspaceLabel.setText("<html><font color=\"" + textColour 
				+ "\">Workspace</font></html>");
		
		initMyspaceEntry(workspaceLabel);
		workspaceLabel.setHorizontalAlignment(SwingConstants.LEFT);
	//	entryPanel.add(workspaceLabel);
		
		return entryPanel;
	}

	/**
	 * Inits a component that is to be added to the MySpace entry list
	 * the size and font will be set
	 * 
	 * @param c the component to init
	 */
	private void initMyspaceEntry(JComponent c) {
		Dimension size = new Dimension(AssetMap.INSTANCE.getWidth(), 50);
		
		c.setMinimumSize(size);
		c.setPreferredSize(size);
		c.setMaximumSize(size);
		c.setFont(new Font("Arial", Font.PLAIN, 14));
	}
	
	/**
	 * Creates A button in the MySource Area. Note that the name will
	 * have <pre><html></html></pre> tags around it. 
	 * 
	 * @param name The name of the button
	 * @param icon the icon for this button
	 * @param l the listener to trigger
	 * @return the new button
	 */
	private JButton createButton(String name, String icon, ActionListener l) {
		JButton button = new JButton("<html>" + name + "</html>");
		
		initMyspaceEntry(button);
		
		button.addActionListener(l);
		button.setIcon(MatrixToolkit.getAssetMapIcon(icon));
		button.setBorder(null);
		
		button.setHorizontalAlignment(SwingConstants.LEFT);
		button.setFocusPainted(false);
		button.setBackground(BUTTON_COLOUR);
		
		return button;
	}
	
	/**
	 * Creates the workspace tree 
	 */
	private JTree createWorkspaceTree() {
		Asset workspace = AssetManager.INSTANCE.getAsset(workspaceId);
		Iterator iterator = workspace.getTreeNodes();
		AssetTreeNode node = (AssetTreeNode) iterator.next();
		ComplexAssetTree workspaceTree = new ComplexAssetTree(new DefaultTreeModel(node));
		workspaceTree.initialise();
		workspaceTree.setRootVisible(true);
		
		workspaceTree.setBorder(new LineBorder(Color.black, 1));
		
		return workspaceTree;
	}
}
