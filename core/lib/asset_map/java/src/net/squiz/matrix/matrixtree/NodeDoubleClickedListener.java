package net.squiz.matrix.matrixtree;

import java.util.EventListener;

/**
 * CueListener defines the interface for an object that listens to
 * double clicks on nodes in associated tree MatrixTree.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public interface NodeDoubleClickedListener extends EventListener {

	/**
	 * Tells listeners that a node has been double clicked.
	 */
	public void nodeDoubleClicked(NodeDoubleClickedEvent e);
}
