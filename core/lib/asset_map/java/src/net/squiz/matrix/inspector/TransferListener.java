package net.squiz.matrix.inspector;

import java.util.EventListener;

/**
 * TransferListener defines the interface for an object that listens to
 * copy or move events in an associated JTable.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public interface TransferListener extends EventListener {

	/**
	 * A transfer gesture has been recognised, and the listening class is
	 * notifying this listener in order for it to initiate the action for
	 * the user
	 */
	public void transferGestureRecognized(TransferEvent e);
}
