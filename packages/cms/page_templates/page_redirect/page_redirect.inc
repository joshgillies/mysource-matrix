<?php

require_once dirname(__FILE__).'/../../page/page.inc';

/**
* Page_Redirect
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
					SQ_LINK_NOTICE => Array('asset' => Array('card' => 'M', 'exclusive' => false)),
				);

	}//end _getAllowedLinks()


	/**
	* Returns the asset (if any) that we are redirecting to
	*
	* @return NULL|object Asset
	* @access public
	*/
	function &getRedirectAsset()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', true, 'redirect_asset');
		if (empty($link)) {
			$null = null;
			return null;
		}

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

	}// end getRedirectAsset()


	/**
	* Set's the asset (if any) that we are going to be redirecting to
	*
	* @param object Asset	the asset that we want to redirect to (NULL clears the redirect asset)
	*
	* @return boolean
	* @access public
	*/
	function setRedirectAsset(&$asset)
	{
		$old_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', true, 'redirect_asset');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (empty($old_link)) {
			// if there is not existing asset, and we aren't setting on, we are done here
			if (is_null($asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				return true;
			}

		// else if there is a link, and we are about to set an asset, but it is the same as we already have
		} elseif (!is_null($asset) && $old_link['minorid'] == $asset->id) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;

		// else something different, delete the old link
		} else {
			if (!$this->deleteLink($old_link['linkid'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		if (!is_null($asset)) {
			if (!$this->createLink($asset, SQ_LINK_NOTICE, 'redirect_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}// end getRedirectAsset()


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
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		// if there is no url, see if we have an asset selected
		if (($url = $this->attr('redirect_url')) == '') {
			$redirect_asset = &$this->getRedirectAsset();
			if (!is_null($redirect_asset)) {
				if (($url = $redirect_asset->getURL()) == '') {
					$url = current_url().'?a='.$redirect_asset->id;
				}
			}
		}// end if

		if ($url == '') {
			trigger_error('No URL or Asset selected for this "'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').'"', E_USER_NOTICE);
			return;
		}

		$url .= trim($this->attr('url_suffix'));

		if ($this->attr('new_window')) {

			$options = $this->attr('new_window_options');
			$params  =	 'location='  .((strpos($options, 'hide_location'  )) ? '0' : '1').','
						.'menubar='   .((strpos($options, 'hide_menubar'   )) ? '0' : '1').','
						.'scrollbars='.((strpos($options, 'hide_scrollbars')) ? '0' : '1').','
						.'status='    .((strpos($options, 'hide_status'    )) ? '0' : '1').','
						.'toolbar='   .((strpos($options, 'hide_toolbar'   )) ? '0' : '1').','
						.'resizable=' .((strpos($options, 'not_resizable'  )) ? '0' : '1');

			if ($w = $this->attr('new_window_width'))  $params .= ',width='.$w;
			if ($h = $this->attr('new_window_height')) $params .= ',height='.$h;

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
			if ($wrap_script) {
				?></body></html><?php
			}

		// not in a new window, just relocate them
		} else {
			if (headers_sent()) {
				?>
				<script language="JavaScript" type="text/javascript">
					self.location = '<?php echo addslashes($url); ?>';
				</script>
				<?php
			} else {
				header('Location: '.$url);
			}

		}

	}// end printFrontend()


	/**
	* Prints out the body of the asset
	*
	* This function just calls printFrotnend()
	*
	* @return void
	* @access public
	* @see printFrontend()
	*/
	function printBody()
	{
		$this->printFrontend();

	}// end printBody()

}//end class

?>