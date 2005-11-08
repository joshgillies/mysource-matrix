package net.squiz.matrix.assetmap;

import javax.swing.*;
import javax.swing.tree.*;
import net.squiz.matrix.matrixtree.*;
import java.awt.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;

public class FinderView extends BasicView {

	protected FinderTree tree;

	public FinderView() {
		construct();
	}

	private void construct() {
		JScrollPane scrollPane = new JScrollPane(createFinderTree());
		scrollPane.setBorder(BorderFactory.createEmptyBorder());

		setLayout(new BorderLayout());
		add(scrollPane);
		add(MatrixStatusBar.createStatusBar(), BorderLayout.SOUTH);
		setSize(300,500);
	}

}
