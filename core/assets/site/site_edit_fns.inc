<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: site_edit_fns.inc,v 1.38 2003/10/21 23:41:38 gsherwood Exp $
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
		?>
		<table border="0" cellspacing="0" cellpadding="3">
		<?php

		$urls = $asset->getSiteURLs();
		$wa = $asset->writeAccess('lookups');

		$urlids = Array();

		for(reset($urls); null !== ($urlid = key($urls)); next($urls)) {
			$this->_paintURL($o, $prefix, $urlid, $urls[$urlid], $wa);
			$urlids[] = $urlid;
		}// end for

		if ($wa) {
			// paint a nice blank version
			$this->_paintURL($o, $prefix, 0, Array('url' => '', 'protocols' => Array()), true);
			$urlids[] = 0;

			// let's be a bit nice about this, and when they submit confirm that they know what they are doing
			$o->addOnSubmit('if (!site_url_check()) return false;');

		}// end if

		?>
		</table>
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
	* @param int						$i				the count for this URL
	* @param Array						$url_info		the urls info
	* @param boolean					$write_access	whether the user can edit this url
	*
	* @return boolean
	* @access public
	*/
	function _paintURL(&$o, $prefix, $i, $url_info, $write_access)
	{
		?>
			<tr>
				<td class="sq-backend-data">
				<?php
					if ($write_access) {
						text_box($prefix.'_urls['.$i.'][url]',  $url_info['url'], 40);
					} else {
						echo $url_info['url'];
					}
				?>
				</td>
		<?php
			for($k = 0; $k < count($this->protos); $k++) {
			?>
				<td class="sq-backend-data">
				<?php
					if ($write_access) {
						check_box($prefix.'_urls['.$i.'][protocols]['.$k.']', $this->protos[$k], !empty($url_info[$this->protos[$k]]));
					} else {
					?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo empty($url_info[$this->protos[$k]]) ? 'cross' : 'tick'; ?>.gif" width="15" height="15" />
					<?php
					}
				?>
				</td>
				<td class="sq-backend-data">
					<?php echo strtoupper($this->protos[$k]); ?>
				</td>
			<?php
			}// end for
		?>
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
				$urls[$urlid] = Array('url' => $post[$urlid]['url']);
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
			}
			echo '</i>';
			return false;
		}

		$pages = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, '', false);

		if (!count($pages)) {
			echo '<i>There are no Pages that are children of this site.</i>';
			hidden_field($prefix.'_'.$page_type, '0');
			return false;
		}

		$contents = Array('' => '');
		foreach($pages as $info) {
			$page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($info['minorid'], $info['minor_type_code'], true);
			if (is_null($page)) continue;
			$contents[$page->id] = $page->name;
		}

		combo_box($prefix.'_'.$page_type, $contents, false, $selected);
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
		if (!empty($link) && $link['minorid'] == $_POST[$prefix.'_'.$page_type]) {
			return true;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// if there is a current link, we need to delete it
		if (!empty($link)) {
			if (!$asset->deleteLink($link['linkid'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}// end if
		}// end if

		if (!empty($_POST[$prefix.'_'.$page_type])) {
			$page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$prefix.'_'.$page_type]);
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