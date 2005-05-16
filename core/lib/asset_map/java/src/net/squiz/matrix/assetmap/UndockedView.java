
package net.squiz.matrix.assetmap;

import javax.swing.*;
import java.awt.event.*;
import net.squiz.matrix.matrixtree.*;

public class UndockedView extends JFrame implements View {

	private View view;
	private int index;

	public UndockedView(View view, int index) {
		this.view = view;
		this.index = index;

		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		setTitle(view.getName());
		getContentPane().add(view.getViewComponent());
	}

//	private JPanel createTools() {
//
//	}

	public MatrixTree getTree() {
		return view.getTree();
	}

	public View getInnerView() {
		return view;
	}

	public int getIndex() {
		return index;
	}

	public String getName() {
		return view.getName();
	}

	public void setName(String name) {
		view.setName(name);
	}

	public JComponent getViewComponent() {
		return view.getViewComponent();
	}
}
