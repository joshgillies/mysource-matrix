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
* $Id: site_edit_fns.inc,v 1.41.2.2 2004/03/16 16:55:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Site_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Site_Edit_Fns extends Folder_Edit_Fns
{
	/**
	* All available protocols
	* @var Array(string)
	*/
	var $protos = Array('http', 'https');

	/**
	* All available scopes
	* @var Array(string)
	*/
	var $system_scopes = Array('G' => 'Global', 'L' => 'Local');


	/**
	* Constructor
	*
	*/
	function Site_Edit_Fns()
	{
		$this->Folder_Edit_Fns();
		$this->static_screens['details']['lock_type'] = 'attr_links';
		unset($this->static_screens['dependants']);

	}//end constructor


	/**
	* Used in a show_if in the interface xml, returns true is server rep is disabled for this system
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function serverRepDisabled(&$asset)
	{
		require_once SQ_SYSTEM_ROOT.'/data/private/conf/replication.inc';
		return !SQ_REPLICATION_ENABLED;

	}// end serverRepDisabled()


	/**
	* Used in a show_if in the interface xml, returns true is server rep is enabled for this system
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function serverRepEnabled(&$asset)
	{
		require_once SQ_SYSTEM_ROOT.'/data/private/conf/replication.inc';
		return SQ_REPLICATION_ENABLED;

	}// end serverRepEnabled()


	/**
	* Paint the interface for the asset urls
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintURLs(&$asset, &$o, $prefix)
	{
		require_once SQ_SYSTEM_ROOT.'/data/private/conf/replication.inc';
		?>
		<table class="sq-backend-table">
			<tr>
		<?php
			if (SQ_REPLICATION_ENABLED) {
			?>
				<td class="sq-backend-table-header" style="white-space: nowrap;">System Scope</td>
			<?php
			}
		?>
				<td class="sq-backend-table-header">Url</td>
				<td class="sq-backend-table-header" width="100%">Protocols</td>
			</tr>
		<?php

		$urls = $asset->getSiteURLs(true);
		$wa = $asset->writeAccess('lookups');

		$urlids = Array();

		for($i = 0; $i < count($urls); $i++) {
			// can only edit urls that are for this system
			$editable = ($wa && SQ_CONF_SYSTEM_ID == $urls[$i]['systemid']);
			$this->_paintURL($o, $prefix, $urls[$i], $editable, SQ_REPLICATION_ENABLED);
			if ($editable) $urlids[] = $urls[$i]['urlid'];
		}// end for

		if ($wa) {
			// paint a nice blank version
			$this->_paintURL($o, $prefix, Array('urlid' => '0', 'url' => '', 'protocols' => Array(), 'system_scope' => 'L'), true, SQ_REPLICATION_ENABLED);
			$urlids[] = '0';

			// let's be a bit nice about this, and when they submit confirm that they know what they are doing
			$o->addOnSubmit('if (!site_url_check()) return false;');

		}// end if

		?>
		</table>
		<?php

		trigger_error('EXPLAIN GLOBAL AND LOCAL SCOPE HERE');
		#$o->note('...');

		?>
		<script language="JavaScript" type="text/javascript">

			var site_url_protos = new Array("<?php echo implode('","', $this->protos); ?>");
			var site_url_urlids = new Array("<?php echo implode('","', $urlids); ?>");

			function site_url_check() {

				for(var j = 0; j < site_url_urlids.length; j++) {
					var urlid = site_url_urlids[j];
					// if there isn't any value in the url box, it doesn't matter
					var url = get_form_element_value("<?php echo $prefix; ?>_urls[" + urlid + "][url]");
					if (url.length) {
						var found = false;
						for(var i = 0; i < site_url_protos.length; i++) {
							if (get_form_element_value("<?php echo $prefix; ?>_urls[" + urlid + "][protocols][" + i + "]")) {
								found = true;
								break;
							}
						}// end if

						// if nothing is checked, confirm deletion
						if (!found) {
							// if they don't want to delete, abort submit
							if (!confirm('You need to select at least one protocol, otherwise the url will be deleted.\nDo you want to delete "' + url + '" ?')) {
								return false;
							}
						}

					// remove all the protocols
					} else {
						for(var i = 0; i < site_url_protos.length; i++) {
							var proto = get_form_element("<?php echo $prefix; ?>_urls[" + urlid + "][protocols][" + i + "]");
							proto.checked = false;
						}// end if

					}// end if

				}// end for

				return true;

			}// end site_url_check();

		</script>
		<?php

		return $wa;

	}//end paintURLs()


	/**
	* Paint the interface for individual URLs
	*
	* @param object	Backend_Outputter	$o				the outputter class
	* @param string						$prefix			prefix for the form element
	* @param Array						$url_info		the urls info
	* @param boolean					$write_access	whether the user can edit this url
	* @param boolean					$show_scope		whether to show the system scope for this URL
	*
	* @return boolean
	* @access public
	*/
	function _paintURL(&$o, $prefix, $url_info, $write_access, $show_scope)
	{
		?>
			<tr>
		<?php
			if ($show_scope) {
		?>
				<td class="sq-backend-table-cell">
				<?php
					if ($write_access) {
						combo_box($prefix.'_urls['.$url_info['urlid'].'][system_scope]', $this->system_scopes, false, $url_info['system_scope']);
					} else {
						echo $this->system_scopes[$url_info['system_scope']];
					}
					if (isset($url_info['systemid'])) echo '<br>(System #'.$url_info['systemid'].')';
				?>
				</td>
		<?php
			} elseif ($write_access) {
				hidden_field($prefix.'_urls['.$url_info['urlid'].'][system_scope]', $url_info['system_scope']);
			}// end if
		?>
				<td class="sq-backend-table-cell">
				<?php
					if ($write_access) {
						text_box($prefix.'_urls['.$url_info['urlid'].'][url]',  $url_info['url'], 40);
					} else {
						echo $url_info['url'];
					}
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
					for($k = 0; $k < count($this->protos); $k++) {
						if ($write_access) {
							check_box($prefix.'_urls['.$url_info['urlid'].'][protocols]['.$k.']', $this->protos[$k], !empty($url_info[$this->protos[$k]]));
						} else {
						?>
							<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo empty($url_info[$this->protos[$k]]) ? 'cross' : 'tick'; ?>.gif" width="15" height="15" />
						<?php
						}
						?>
						&nbsp;<?php echo strtoupper($this->protos[$k]); ?>&nbsp;&nbsp;
					<?php
					}// end for
					?>
				</td>
			</tr>
		<?php

	}//end _paintURL()


	/**
	* Processes the URLs posted from the paintURLs fn above
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processURLs(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_urls'])) return false;

		$post = $_POST[$prefix.'_urls'];

		$urls = Array();
		for(reset($post); null !== ($urlid = key($post)); next($post)) {

			$url = trim($post[$urlid]['url']);
			// if there is a url or some selected protocols, then let's save it
			if ($url && !empty($post[$urlid]['protocols'])) {
				$urls[$urlid] = Array('url' => $post[$urlid]['url'], 'system_scope' => $post[$urlid]['system_scope']);
				for($k = 0; $k < count($this->protos); $k++) {
					$urls[$urlid][$this->protos[$k]] = (int) in_array($this->protos[$k], $post[$urlid]['protocols']);
				}
			}

		}

		if (!$asset->saveWebURLs($urls)) return false;

		//// START HIPO PROCESSING ////
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($asset->id));
		$hh->queueHipo('hipo_job_update_lookups', $vars);
		////  END HIPO PROCESSING  ////

		return true;

	}//end processURLS()


	/**
	* Paint the interface for setting the $page_type page for this site
	*
	* @param string			$page_type	the type of page this is, ie the string in the value field of the link
	* @param object Asset	$owner		the asset whose interface we are painting
	* @param string			$prefix		prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintPageSelection($page_type, &$asset, $prefix)
	{
		$link = $asset->getSpecialPageLink($page_type);
		$selected = (empty($link)) ? 0 : $link['minorid'];

		if (!$asset->writeAccess('links')) {
			echo '<i>';
			if (!$selected) {
				echo 'None';
			} else {
				$page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($selected, '', true);
				echo $page->name;
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($page);
			}
			echo '</i>';
			return false;
		}
		
		asset_finder($prefix.'_'.$page_type, $selected);
		
		return true;

	}//end paintPageSelection()


	/**
	* Processes the setting of the index page for this site
	*
	* @param string			$page_type	the type of page this is, ie the string in the value field of the link
	* @param object Asset	$owner	the asset whose interface we are painting
	* @param string			$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPageSelection($page_type, &$asset, $prefix)
	{
		if (!isset($_POST[$prefix.'_'.$page_type])) return false;

		$link = $asset->getSpecialPageLink($page_type);
		// if there has been no change, then there is nothing to do
		if (!empty($link) && $link['minorid'] == $_POST[$prefix.'_'.$page_type]['assetid']) {
			return true;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// if there is a current link, we need to delete it
		if (!empty($link)) {
			if (!$asset->deleteLink($link['linkid'])) {
				bam('could not delete link');
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}// end if
		}// end if

		if (!empty($_POST[$prefix.'_'.$page_type]['assetid'])) {
			$page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$prefix.'_'.$page_type]['assetid']);

			if (!$asset->createLink($page, SQ_LINK_TYPE_3, $page_type)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}// end if
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processPageSelection()


	/**
	* Paint the interface for setting the index page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintIndexPage(&$asset, &$o, $prefix)
	{
		$this->paintPageSelection('index', $asset, $prefix);

	}//end paintIndexPage


	/**
	* Processes the setting of the index page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processIndexPage(&$asset, &$o, $prefix)
	{
		$this->processPageSelection('index', $asset, $prefix);

	}//end processIndexPage()


	/**
	* Paint the interface for setting the forbidden page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintForbiddenPage(&$asset, &$o, $prefix)
	{
		$this->paintPageSelection('forbidden', $asset, $prefix);

	}//end paintForbiddenPage


	/**
	* Processes the setting of the forbidden page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processForbiddenPage(&$asset, &$o, $prefix)
	{
		$this->processPageSelection('forbidden', $asset, $prefix);

	}//end processForbiddenPage()


	/**
	* Paint the interface for setting the not found page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintNotFoundPage(&$asset, &$o, $prefix)
	{
		$this->paintPageSelection('not_found', $asset, $prefix);

	}//end paintNotFoundPage


	/**
	* Processes the setting of the not found page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNotFoundPage(&$asset, &$o, $prefix)
	{
		$this->processPageSelection('not_found', $asset, $prefix);

	}//end processNotFoundPage()

}//end class

?>