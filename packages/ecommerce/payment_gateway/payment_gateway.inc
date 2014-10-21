<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: payment_gateway.inc,v 1.14 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Payment_Gateway
*
* Abstract asset for other payment gateways to extend
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway extends Page
{


	/**
	* Create this asset
	*
	* The return value will be:<br/>
	* <ul>
	*   <li>FALSE if the asset was not created</li>
	*   <li>the ID of the newly created link if the asset and intital link were created</li>
	* </ul>
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </pre>
	*
	* @return mixed int|boolean
	* @access public
	*/
	function create(&$link)
	{
		if ($this->attr('short_name') == '') {
			$this->setAttrValue('short_name', $this->attr('name'));
		}
		return parent::create($link);

	}//end create()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// set an initial web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
		return $this->saveWebPaths($good_paths);

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$attr = ($short_name) ? 'short_name' : 'name';
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr, $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Append a message to the Audit History Log of the currently-pending Order
	*
	* @param $msg	The message to append
	* @returns void
	* @access protected
	*/
	protected function appendPendingOrderAuditLog($msg)
	{
		if (isset($_SESSION['sq_local_cart_contents']['pending_order_id'])) {
			$pending_order_id = $_SESSION['sq_local_cart_contents']['pending_order_id'];
			$pending_order = $GLOBALS['SQ_SYSTEM']->am->getAsset($pending_order_id);

			$pending_order->appendAuditLog($msg);
		}
	}


	/**
	 * Set the merchant reference number to the currently-pending Order
	 *
	 * @param string $ref_no	The merchant reference number to set
	 * @returns void
	 * @access protected
	 */
	protected function setPendingOrderMerchantReference($ref_no)
	{
		if (isset($_SESSION['sq_local_cart_contents']['pending_order_id'])) {
			$pending_order = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['sq_local_cart_contents']['pending_order_id']);

			$pending_order->setAttrValue('ecom_ref_no', $ref_no);
			$pending_order->appendAuditLog(translate('Order reference number recorded before processing'));

		}

	}//end setPendingOrderMerchantReference()



	/**
	* Determine whether this asset should send cacheable header
	*
	*
	* @return boolean
	* @access public
	*/
	public function canSendCacheableHeader()
	{
		return FALSE;
	}//end canSendCacheableHeader()



}//end class

?>
