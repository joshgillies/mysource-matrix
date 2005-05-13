package net.squiz.matrix.assetmap;

import javax.swing.*;
import javax.swing.tree.*;
import net.squiz.matrix.matrixtree.*;
import java.awt.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;

public class FinderView extends JPanel {

	protected FinderTree tree;

	public FinderView() {
		construct();
	}

	private void construct() {
		JScrollPane scrollPane = new JScrollPane(constructTree());
		scrollPane.setBorder(BorderFactory.createEmptyBorder());

		setLayout(new BorderLayout());
		add(scrollPane);
		add(MatrixStatusBar.createStatusBar(), BorderLayout.SOUTH);
		setSize(300,500);
	}

	protected MatrixTree constructTree() {
		tree = MatrixTreeBus.createFinderTree(new LoadingNode());
		return tree;
	}

	public FinderTree getTree() {
		return tree;
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
