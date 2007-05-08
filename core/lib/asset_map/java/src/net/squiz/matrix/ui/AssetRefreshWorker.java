/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: AssetRefreshWorker.java,v 1.7 2007/05/08 02:28:37 rong Exp $
*
*/

package net.squiz.matrix.ui;

import net.squiz.matrix.matrixtree.MatrixTreeNode;
import net.squiz.matrix.core.*;
import net.squiz.matrix.debug.*;
import org.w3c.dom.*;
import java.io.IOException;


/**
 * The AssetRefreshWorker makes a request to the Matrix system for the specified
 * assets in a separate thread, then refresh the assets in the EventDispachThread.
 * You should use this class over all other methods of refreshing as it correctly
 * obeys the swing thread laws.
 *
 * Example:
 * <pre>
 *    AssetRefreshWorker worker = new AssetRefreshWorker(assetid, true);
 *    worker.start();
 * </pre>
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetRefreshWorker extends MatrixSwingWorker {

	private String[] assetids;
	private MatrixTreeNode node;
	private boolean throwVisibleError = false;

	/**
	 * Refreshes all known assets.
	 * @param throwVisibleError if TRUE an dialog error is thrown if one occurs
	 */
	public AssetRefreshWorker(boolean throwVisibleError) {
		assetids = AssetManager.getAllRefreshableAssetids();
		this.throwVisibleError = throwVisibleError;
	}

	/**
	 * Refreshes the specified assets.
	 * @param assetids the assetids to refresh
	 * @param throwVisibleError if TRUE an dialog error is thrown if one occurs
	 */
	public AssetRefreshWorker(String[] assetids, boolean throwVisibleError) {
		this.assetids          = assetids;
		this.throwVisibleError = throwVisibleError;
	}

	/**
	 * Refreshes the specified node and its children.
	 * @param node the node to refresh
	 * @param throwVisibleError if TRUE an dialog error is thrown if one occurs
	 */
	public AssetRefreshWorker(MatrixTreeNode node, boolean throwVisibleError) {
		this.node              = node;
		this.assetids          = new String[] { node.getAsset().getId() };
		this.throwVisibleError = throwVisibleError;
	}

	/**
	 * Constructs the worker and returns the object to be used by get(). No
	 * GUI updates should be executed in the method. GUI updates should occur
	 * in finished()
	 * @return the object to get obtained by get()
	 * @see MatrixSwingWorker.get()
	 * @see MatrixSwingWorker.construct()
	 * @see finished()
	 * @see MatrixSwingWorker.finished()
	 */
	public Object construct() {
		try {
			return AssetManager.makeRefreshRequest(assetids, "");
		} catch(IOException ioe) {
			return ioe;
		}
	}

	/**
	 * Called from the EventDispachThread once construct has completed.
	 * @see construct()
	 * @see MatrixSwingWorker.finished()
	 */
	public void finished() {
		Object get = get();

		// success!
		if (get instanceof Element) {
			Element element = (Element) get;
			AssetManager.refreshAssets(element);

			MatrixStatusBar.setStatusAndClear(
				getStatusBarSuccessMessage(),
				getStatusClearTime()
			);
		// we have an error
		} else if (get instanceof IOException) {
			IOException ioe = (IOException) get;
			String message = getErrorMessage() +":" + ioe.getMessage();
			if (throwVisibleError) {
				GUIUtilities.error(null, message, getErrorTitle());
			}
			MatrixStatusBar.setStatusAndClear(
				getStatusBarFailMessage(),
				getStatusClearTime()
			);
			Log.log(message, AssetRefreshWorker.class, ioe);
		}
	}

	/**
	 * Starts the worker.
	 * @see MatrixSwingWorker.start()
	 */
	public void start() {
		MatrixStatusBar.setStatus(getStatusBarWaitMessage());
		super.start();
	}

	/**
	 * Returns the message used when in the status bar when an error occurs.
	 * By default the error is 'Failed!'
	 * @return the error message
	 */
	protected String getStatusBarFailMessage() { return Matrix.translate("asset_map_status_bar_failed"); }

	/**
	 * Returns the message used when in the status bar when no error occurs.
	 * By default the message is 'Success!'
	 * @return the success message
	 */
	protected String getStatusBarSuccessMessage() { return Matrix.translate("asset_map_status_bar_success"); }

	/**
	 * Returns the message used while waiting for construct() to return
	 * By default the message is 'Requesting...'
	 * @return the waiting message
	 */
	protected String getStatusBarWaitMessage() { return Matrix.translate("asset_map_status_bar_requesting"); }

	/**
	 * Returns the message used in the dialog error popup
	 * By default the error is 'Error While Requesting' followed by the error
	 * @return the error message
	 */
	protected String getErrorMessage() { return Matrix.translate("asset_map_status_bar_error_requesting"); }

	/**
	 * Returns the message used in the dialog title when an error occurs
	 * By default the error is 'Error Refreshing Assets'
	 * @return the error message
	 */
	protected String getErrorTitle() { return Matrix.translate("asset_map_status_bar_error_refreshing"); }

	/**
	 * Returns the message used logging an error message.
	 * By default this method calls getErrorMessage()
	 * @return the error message
	 */
	protected String getLogMessage() { return getErrorMessage(); }

	/**
	 * Returns the duration in milliseconds that the status bar waits before
	 * clearing the success or error message. By default this is set to 1000 ms.
	 * @return the duration to wait before clearing the message
	 */
	protected int getStatusClearTime() { return 1000; }
}
