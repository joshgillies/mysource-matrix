<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_redirect.inc,v 1.27 2006/06/06 04:55:29 lwright Exp $
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
* @version $Revision: 1.27 $
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
	function Page_Redirect($assetid=0)
	{
		$this->Page($assetid);

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
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'redirect_asset');
		if (empty($link)) {
			$null = NULL;
			return $null;
		}

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

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
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', translate('cannot_access_asset', $this->name));
			return;
		}

		$url = $this->_getRedirectURL();

		$timeout = $this->attr('timeout');
		if ($timeout > 0) {
			parent::printFrontend();
			echo $this->getJavascriptRedirectKeywordReplacement();
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
			if (!headers_sent()) {
				header('Location: '.$url);
			} else {
				parent::printFrontend();
				echo $this->getJavascriptRedirectKeywordReplacement();
			}
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

		$asset =& new Asset();

		foreach ($asset->getAvailableKeywords() as $keyword => $desc) {
			$keywords['referring_'.$keyword] = 'Referring Asset: '.$desc;
		}

		$keywords['javascript_redirect'] = 'JavaScript Redirect Code';

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
		if (preg_match('|^referring\_(.*)|', $keyword, $matches)) {
			$referring_asset =& $this->getReferringAsset();
			if ($referring_asset) {
				return $referring_asset->getKeywordReplacement($matches[1]);
			} else {
				return '';
			}
		} else {
			return parent::getKeywordReplacement($keyword);
		}

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

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, $url, FALSE, TRUE);

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
		$url = '';

		// See if we are given a query string value to look at
		$qrystr_param = $this->attr('use_querystring_parameter');
		if ($qrystr_param && isset($_GET[$qrystr_param]) && trim($_GET[$qrystr_param]) != '') {
			$url = trim($_GET[$qrystr_param]);

		// if there is no url, see if we have an asset selected
		} else if (($url = $this->attr('redirect_url')) == '') {
			$redirect_asset =& $this->getRedirectAsset();
			if (!is_null($redirect_asset)) {
				if (($url = $redirect_asset->getURL()) == '') {
					$url = current_url().'?a='.$redirect_asset->id;
				}
			}
		}//end if

		if ($url == '') {
			trigger_localised_error('CMS0037', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return;
		}

		$url .= trim($this->attr('url_suffix'));

		// See if we need to append the query string and there is a query string
		if ($this->attr('append_querystring') && !empty($_SERVER['QUERY_STRING'])) {
			$url .= ((strpos($url, '?') === FALSE) ? '?' : '&').$_SERVER['QUERY_STRING'];
		}

		return $url;

	}//end _getRedirectURL()


	/**
	* Get "%javascript_redirect% keyword replacement
	*
	* @return string
	* @access public
	*/
	function getJavascriptRedirectKeywordReplacement()
	{
		static $fn_called = FALSE;

		if ($fn_called) return '';
		$fn_called = TRUE;

		$timeout = $this->attr('timeout');
		$url = $this->_getRedirectURL();

		ob_start();
			if (headers_sent()) {
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
