
package net.squiz.matrix.assetmap;

import javax.swing.*;
import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.inspector.*;
import java.awt.*;
import java.awt.event.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
import javax.swing.plaf.*;
import net.squiz.matrix.plaf.*;

public class BasicView extends JPanel implements View {

	protected MatrixTree tree;
	protected Inspector inspector;
	private JSplitPane pane;

	public BasicView() {
		construct();
	}

	private void construct() {
		pane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
		JScrollPane scrollPane = new JScrollPane(constructTree());
		// MM: quick hack because the L&F does not seem to want to instantiate
		// MatrixSplitPaneUI.....bitch
		inspector = constructInspector();
		pane.setUI(new MatrixSplitPaneUI());

		scrollPane.setBorder(BorderFactory.createEmptyBorder());
		pane.setTopComponent(scrollPane);
		pane.setBottomComponent(inspector);
		pane.setResizeWeight(.66D);
		pane.setDividerLocation(Integer.MAX_VALUE);
		pane.setLastDividerLocation((int)(AssetMap.getApplet().getHeight() * 0.55));
		pane.setOneTouchExpandable(true);
		SplitPaneUI ui = pane.getUI();


		// add a listener so that we can listen when the divider has been
		// docked at the very bottom of the view. When this happens, we want
		// to set the tree to expand nodes on double clicks
		if (ui instanceof MatrixSplitPaneUI) {
			MatrixSplitPaneDivider divider = (MatrixSplitPaneDivider) ((MatrixSplitPaneUI) ui).getDivider();
			divider.addMouseListener(new OneTouchMouseHandler());
			divider.addRightOneTouchExpandableListener(new OneTouchActionHandler(false));
			divider.addLeftOneTouchExpandableListener(new OneTouchActionHandler(true));
		}

		setLayout(new BorderLayout());
		add(new AssetMapMenuPanel(tree, inspector.getInspectorGadget()), BorderLayout.NORTH);
		add(MatrixStatusBar.createStatusBar(), BorderLayout.SOUTH);
		add(pane);
	}

	protected MatrixTree constructTree() {
		tree = MatrixTreeBus.createTree(new LoadingNode());
		return tree;
	}

	private class OneTouchMouseHandler implements MouseListener {

		public void mouseClicked(MouseEvent e) {}
		public void mousePressed(MouseEvent e) {}
		public void mouseReleased(MouseEvent e) {
			tree.addNodeDoubleClickedListener(inspector.getInspectorGadget());
			tree.setToggleClickCount(1000);
		}
		public void mouseEntered(MouseEvent e) {}
		public void mouseExited(MouseEvent e) {}
	}

	private class OneTouchActionHandler implements ActionListener {

		private boolean toMinimum;

		OneTouchActionHandler(boolean toMinimum) {
			this.toMinimum = toMinimum;
		}

		public void actionPerformed(ActionEvent e) {
			SplitPaneUI splitPaneUI = pane.getUI();
			MatrixSplitPaneDivider divider
				= (MatrixSplitPaneDivider) ((MatrixSplitPaneUI) splitPaneUI).getDivider();
			Insets  insets = pane.getInsets();
			int lastLoc = pane.getLastDividerLocation();

			int currentLoc = splitPaneUI.getDividerLocation(pane);
			int newLoc;

			if (toMinimum) {
				// we are expanded up to the middle
				if (currentLoc >= (pane.getHeight() - insets.bottom - divider.getHeight())) {
					tree.addNodeDoubleClickedListener(inspector.getInspectorGadget());
					tree.setToggleClickCount(1000);
				}
			} else {
				// we have expanded down to the bottom
				if (currentLoc != insets.top) {
					tree.removeNodeDoubleClickedListener(inspector.getInspectorGadget());
					tree.setToggleClickCount(2);
				}
			}
		}
	}// end class OneTouchActionHandler

	protected Inspector constructInspector() {
		inspector = new Inspector(tree);

		return inspector;
	}

	public MatrixTree getTree() {
		return tree;
	}

	public Inspector getInspector() {
		return inspector;
	}

	public String getName() {
		return getAccessibleContext().getAccessibleName();
	}

	public void setName(String name) {
		getAccessibleContext().setAccessibleName(name);
	}

	public JComponent getViewComponent() {
		return this;
	}
}
