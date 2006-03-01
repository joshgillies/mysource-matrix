package net.squiz.matrix.assetmap;

import javax.swing.*;
import javax.swing.tree.*;
import net.squiz.matrix.matrixtree.*;
import java.awt.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
import javax.swing.plaf.*;
import net.squiz.matrix.plaf.*;

public class FinderView extends BasicView {

	protected FinderTree tree;

	public FinderView() {
		construct();
	}

	private void construct() {
		tree = (FinderTree)createFinderTree();
		JScrollPane scrollPane = new JScrollPane(tree);
		scrollPane.setBorder(BorderFactory.createEmptyBorder());

		setLayout(new BorderLayout());
		add(scrollPane);
		add(MatrixStatusBar.createStatusBar(), BorderLayout.SOUTH);
		add(new AssetMapMenuPanel(tree, false), BorderLayout.NORTH);
		setSize(300,500);
	}

}
