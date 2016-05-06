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
	* The pass through variable array
	* @var array
	*/
	private $_pass_through_vars = NULL;


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
	* Returns the additional files directory path
	* loader.js and loader.gif are in this directory
	*
	* @return string
	* @access public
	*/
	public function additionalFilesDirPath()
	{
		return sq_web_path('data').'/asset_types/payment_gateway/files';

	}//end additionalFilesDirPath()


	/**
	* Include the "Loader" JS object 
	* Used for showing the overlay screen when payment form is submitted
	*
	* @param string $prefix
	*
	* @return string
	* @access public
	*/
	public function includeLoaderJsObject($prefix='')
	{
		$path = $this->additionalFilesDirPath();
		if (empty($prefix)) {
			$prefix = $this->getPrefix().'_card';
		}

		?>
			<link rel="stylesheet" type="text/css" href="<?php echo $path; ?>/loader.css">
			<script src="<?php echo $path; ?>/loader.js"></script>
			<script>
				var Loader = new Loader('<?php echo $prefix; ?>','#FFFFFF','Processing Transaction...', '<?php echo $path; ?>/loader.gif');
				Loader.print();
			</script>
		<?php

	}//end includeLoaderJsObject()

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


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access public
	*/
	public function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$keywords = $this->getAvailableKeywords();
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Return the bodycopy linked to this page
	*
	* @return object
	* @access public
	*/
	public function getBodycopy($link_value='')
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $link_value);
		if (empty($link)) return NULL;
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

		return $bodycopy;

	}//end getBodycopy()


	/**
	* Create Bodycopy
	*
	* @param string	$link_value		the link value of the link between this Datacash Payment Gateway asset and the new bodycopy
	* @param string	$content		the content to put into the bodycopy
	*
	* @return string
	* @access protected 
	*/
	protected function _createBodycopy($link_value, $content)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => $link_value);

		$bodycopy->setAttrValue('name', ucwords(str_replace('_',' ', $link_value)));
		$args = Array('content' => $content);
		if ($bodycopy->create($copy_link, $args)) {
			$child_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_div');
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($child_bodycopy['minorid'], 'attributes');
			return TRUE;
		}
		return FALSE;

	}//end _createBodycopy()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access public
	* @see Asset::_getAllowedLinks()
	*/
	public function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 2,   'exclusive' => TRUE);
		return $page_links;

	}//end _getAllowedLinks()

	
	/**
	* Get pass-through variable which is stored in a Session variable
	*
	* @param string    $var_name   the name of the pass-through variable
	* @param mixed     $default    the default value to get if the $var_name variable is not in the Session
	*
	* @return mixed
	* @access protected
	*/
	protected function _getPassThroughVariable($var_name, $default)
	{
		if (is_null($this->_pass_through_vars)) {
			$this->_pass_through_vars = array_get_index($_SESSION, 'SQ_ECOM_PASS_THROUGH_VARS', Array());
		}

		return array_get_index($this->_pass_through_vars, $var_name, $default);

	}//end _getPassThroughVariable()


	/**
	* Get pass-through variable with the variable names from attributes
	*
	* @param string    $attr_name      the name of the attribute that stores the variable name
	*
	* @return mixed
	* @access protected
	*/
	protected function _getDefaultInputFromPassThroughAttribute($attr_name)
	{
		$result = '';
		$var_name = isset($this->vars[$attr_name]) ? $this->attr($attr_name) : '';
		if (!empty($var_name)) {
			$result = $this->_getPassThroughVariable($var_name, '');
		}

		return $result;

	}//end _getDefaultInputFromPassThroughAttribute()
	
	
	/**
	 * Set error for the transaction so that the getProcessingErrorKeywordReplacement() method will return the error message
	 *
	 * @param string $err_message	the error message to set
	 *
	 * @return void
	 * @access protected
	 */
	protected function _setError($err_message)
	{
		$this->_tmp['is_error'] = TRUE;
		$this->_tmp['error_message'][] = $err_message;

	}//end _setError()


	/**
	 * Check if the transaction has error
	 *
	 * @return boolean
	 * @access public
	 */
	public function isError()
	{
		return array_get_index($this->_tmp, 'is_error', FALSE);

	}//end isError()


	/**
	 * Get the error message under HTML format
	 *
	 * @return string		the HTML error message
	 * @access public
	 */
	public function getErrorMessage()
	{
		$err_message = '';
		$errors = array_get_index($this->_tmp, 'error_message', Array());
		foreach ($errors as $error) {
			$err_message .= "<li>".htmlspecialchars($error, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET)." </li>";
		}

		return '<ul>'.$err_message.'</ul>';

	}//end getErrorMessage()


}//end class

?>
