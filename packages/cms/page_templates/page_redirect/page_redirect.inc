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
* $Id: page_redirect.inc,v 1.47 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Page_Redirect
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.47 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Redirect extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['redirect'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Returns the asset (if any) that we are redirecting to
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getRedirectAsset()
	{
		$res = NULL;
		if ($this->useSystemVersion()) {
			$notice_links = unserialize(file_to_string($this->data_path.'/.sq_notice_links'));
			foreach ($notice_links as $link) {
				if ($link['value'] == 'redirect_asset') {
					$redirect_link = $link;
					break;
				}
			}
		} else {
			if (!empty($this->id)) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'redirect_asset');
			}
		}
		if (!empty($link)) {
			$res = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		}

		return $res;

	}//end getRedirectAsset()


	/**
	* Set's the asset (if any) that we are going to be redirecting to
	*
	* @param object	&$asset	the asset that we want to redirect to (NULL clears the redirect asset)
	*
	* @return boolean
	* @access public
	*/
	function setRedirectAsset(&$asset)
	{
		$old_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'redirect_asset');
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (empty($old_link)) {
			// if there is not existing asset, and we aren't setting on, we are done here
			if (is_null($asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return TRUE;
			}

		// else if there is a link, and we are about to set an asset, but it is the same as we already have
		} else if (!is_null($asset) && $old_link['minorid'] == $asset->id) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;

		// else something different, delete the old link
		} else {
			if (!$this->deleteLink($old_link['linkid'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		if (!is_null($asset)) {
			if (!$this->createLink($asset, SQ_LINK_NOTICE, 'redirect_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end setRedirectAsset()


	/**
	* Prints out the Frontend for the asset
	*
	* The user is redirected to another location, so no design is printed
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{

		// check that the user has read access to the site
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', sprintf(translate('You do not have permission to access the asset'), $this->name));

			return;
		}//end if

		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', FALSE);

		if (SQ_IN_LIMBO) {
			// if we are in LIMBO then do not redirect
			$this->printFrontendAsset($this);
			return;
		}//end if

		$url = $this->_getRedirectURL();
		if (empty($url)) return;

		// Set the Session variables
		$pmap = $this->getAttribute('session_variables');
		$pmap_value = $pmap->getParameters();
		$return_value = '';
		foreach ($pmap_value as $index => $value) {
			if (isset($_SESSION[SQ_SESSION_SANDBOX_INDEX][$value])) {
				unset($_SESSION[SQ_SESSION_SANDBOX_INDEX][$value]);
			}//end if

			// Get the value
			$return_value = $pmap->getParameterValue($value);

			// Replace global keywords if required
			replace_global_keywords($return_value);

			// Set the SESSION
			$_SESSION[SQ_SESSION_SANDBOX_INDEX][$value] = $return_value;
		}//end foreach

		//if redirect permanently, change location now
		if($this->attr('permanent_redirect') && !headers_sent()) {
				header('HTTP/1.1 301 Moved Permanently');
				header('Location: '.$url);
				return;
		}

		$timeout = $this->attr('timeout');
		if (($timeout > 0) || headers_sent()) {
			if (!isset($this->_tmp['parent_already_called'])) {
				$this->_tmp['parent_already_called'] = TRUE;
				parent::printFrontend();
			}
			echo $this->getJavascriptRedirectKeywordReplacement(TRUE);
		}

		if ($this->attr('new_window')) {
			$options = $this->attr('new_window_options');
			$params  =	 'location='.((strpos($options, 'hide_location'  ) !== FALSE) ? '0' : '1').','
						.'menubar='.((strpos($options, 'hide_menubar'   ) !== FALSE) ? '0' : '1').','
						.'scrollbars='.((strpos($options, 'hide_scrollbars') !== FALSE) ? '0' : '1').','
						.'statusbar='.((strpos($options, 'hide_status'    ) !== FALSE) ? '0' : '1').','
						.'toolbar='.((strpos($options, 'hide_toolbar'   ) !== FALSE) ? '0' : '1').','
						.'resizable='.((strpos($options, 'not_resizable'  ) !== FALSE) ? '0' : '1');
			if ($w = $this->attr('new_window_width')) {
				$params .= ',width='.$w;
			}
			if ($h = $this->attr('new_window_height')) {
				$params .= ',height='.$h;
			}

			// if the headers have been sent then other stuff
			$wrap_script = !headers_sent();
			if ($wrap_script) {
				?><html><body><?php
			}
			?>
				<script language="JavaScript" type="text/javascript">
					var page = window.open('<?php echo addslashes($url); ?>','<?php echo time(); ?>','<?php echo addslashes($params);?>');
					history.go(-1);
				</script>
			<?php
			if ($wrap_script) ?></body></html><?php

		// not in a new window, just relocate them
		} else {
			if (!$timeout && !headers_sent()) header('Location: '.$url);
		}

	}//end printFrontend()


	/**
	* Get available keywords
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$asset = new Asset();

		foreach ($asset->getAvailableKeywords() as $keyword => $desc) {
			$keywords['referring_'.$keyword] = 'Referring Asset: '.$desc;
		}

		$keywords['javascript_redirect'] = 'JavaScript Redirect Code';
		$keywords['redirect_url'] = 'Redirect URL';
		$keywords['redirect_assetid'] = 'Redirect Asset Id';

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$replacement = NULL;
		$original_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);

		if (preg_match('|^referring\_(.*)|', $keyword, $matches)) {
			$referring_asset = $this->getReferringAsset();
			$replacement = empty($referring_asset) ? '' : $referring_asset->getKeywordReplacement($matches[1]);
		} else if ($keyword === 'redirect_url') {
			$replacement = $this->_getRedirectURL();
		} else if ($keyword === 'redirect_assetid') {
			$redirect_asset = $this->getRedirectAsset();
			$replacement = empty($redirect_asset) ? '' : $redirect_asset->id;
		} else {
			return parent::getKeywordReplacement($original_keyword);
		}

		if (!is_null($replacement)) {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Gets the referring asset
	*
	* @return object
	* @access public
	*/
	function &getReferringAsset()
	{
		$url = array_get_index($_SERVER, 'HTTP_REFERER', FALSE);

		// strip protocol, strip query string
		$url = preg_replace('|^.*://|', '', $url);
		$url = preg_replace('|\?.*$|', '', $url);

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, $url, FALSE, TRUE);

		return $asset;

	}//end getReferringAsset()


	/**
	* print body
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		echo $this->printFrontend();

	}//end printBody()


	/**
	* get the URL that we will be redirecting to
	*
	* @return string
	* @access private
	*/
	function _getRedirectURL()
	{
		// See if we are given a query string value to look at
		$qrystr_param = $this->attr('use_querystring_parameter');
		$url = trim($this->attr('redirect_url'));
		if ($qrystr_param && isset($_GET[$qrystr_param]) && trim($_GET[$qrystr_param]) != '') {
			$url = trim($_GET[$qrystr_param]);

		// See if there is a URL entered manually in the redirect_url attribute field
		} else if ($url != '') {
			// Replace global keywords
			replace_global_keywords($url);

		// if there is no url, see if we have an asset selected
		} else {
			$redirect_asset = $this->getRedirectAsset();
			if (!is_null($redirect_asset)) {
				if (($url = $redirect_asset->getURL()) == '') {
					$url = current_url().'?a='.$redirect_asset->id;
				}
			}
		}//end if

		if ($url == '') {
			trigger_localised_error('CMS0037', sprintf(translate('No URL or Asset selected for %1$s "%2$s" (Id #%3$s)'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), $this->name, $this->id), E_USER_WARNING);
			return '';
		}

		$url_suffix = trim($this->attr('url_suffix'));

		// Check if URL suffix attribute has a value
		if ($url_suffix != '') {

			// Replace globals keywords on URL suffix
			replace_global_keywords($url_suffix);

			// Append the suffix to the URL
			$url .= $url_suffix;
		}

		// See if we need to append the query string and there is a query string
		if ($this->attr('append_querystring') && !empty($_SERVER['QUERY_STRING'])) {
			$url .= ((strpos($url, '?') === FALSE) ? '?' : '&').$_SERVER['QUERY_STRING'];
		}

		// Strip newlines (and other dodgy characters) from the $url that may have
		// been inadvertently added by keywords or modifiers. They're not allowed in
		// the 'Location' header and will cause an error.
		$stripchars = array();
		for ($i = 0; $i < 32; $stripchars[] = chr($i), $i++);
		$url = str_replace($stripchars, '', $url);

		return $url;

	}//end _getRedirectURL()


	/**
	* Get "%javascript_redirect% keyword replacement
	*
	* @param boolean $force_print 	If TRUE, always returns the js redirection code
	*
	* @return string
	* @access public
	*/
	function getJavascriptRedirectKeywordReplacement($force_print=FALSE)
	{
		static $fn_called = FALSE;

		if ($fn_called) return '';
		$fn_called = TRUE;

		$timeout = $this->attr('timeout');
		$url = $this->_getRedirectURL();
		if (empty($url)) return;

		ob_start();
			if ($force_print || headers_sent()) {
				?>
				<script language="JavaScript" type="text/javascript">
					setTimeout("self.location = '<?php echo addslashes($url); ?>'", <?php echo $timeout * 1000 ?>);
				</script>
				<?php
			}
		return ob_get_clean();

	}//end getJavascriptRedirectKeywordReplacement()


}//end class

?>
