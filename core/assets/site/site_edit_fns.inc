<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Site_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
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
	}

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
		<script language="JavaScript" type="text/javascript">

			var site_url_protos = new Array("<?php echo implode('","', $this->protos); ?>");

			function site_url_url_change(url_num) {

				// if there isn't any value in the url box, it doesn't matter
				if (get_element_value("<?php echo $prefix; ?>_urls[" + url_num + "][url]")) {
					var found = false;
					for(var i = 0; i < site_url_protos.length; i++) {
						if (get_element_value("<?php echo $prefix; ?>_urls[" + url_num + "][protocols][" + i + "]")) {
							found = true;
							break;
						}
					}// end if

					// if nothing is checked, then select the first protocol
					if (!found) {
						var first_proto = get_element("<?php echo $prefix; ?>_urls[" + url_num + "][protocols][0]");
						first_proto.checked = true;
					}

				// remove all the protocols 
				} else {
					for(var i = 0; i < site_url_protos.length; i++) {
						var proto = get_element("<?php echo $prefix; ?>_urls[" + url_num + "][protocols][" + i + "]");
						proto.checked = false;
					}// end if

				}// end if

			}// end site_url_url_change();

			function site_url_protocol_click(url_num) {

				// if there isn't any value in the url box, it doesn't matter
				if (!get_element_value("<?php echo $prefix; ?>_urls[" + url_num + "][url]")) return;

				for(var i = 0; i < site_url_protos.length; i++) {
					if (get_element_value("<?php echo $prefix; ?>_urls[" + url_num + "][protocols][" + i + "]")) return;
				}// end if

				// if we get this far then there is nothing checked
				alert('You need to select at least one protocol, otherwise the url will be deleted');

			}// end site_url_protocol_click();


		</script>
		<table border="0" cellspacing="0" cellpadding="3">
		<?php

		$urls = $asset->getSiteURLs();
#		pre_echo("URLS : ");
#		pre_echo($urls);

		for(reset($urls); null !== ($urlid = key($urls)); next($urls)) {
			$this->_paintURL($o, $prefix, $urlid, $urls[$urlid]) ;
		}// end for

		$this->_paintURL($o, $prefix, 0, Array('url' => '', 'protocols' => Array())) ;
		?>
		</table>
		<?php
		
	}// end paintURLs()


	/**
	* Paint the interface for individual URLs
	*
	* @param object	Backend_Outputter	$o				the outputter class
	* @param string						$prefix			prefix for the form element
	* @param int						$i				the count for this URL
	* @param Array						$url_info		the urls info
	*
	* @return boolean
	* @access public
	*/
	function _paintURL(&$o, $prefix, $i, $url_info) 
	{

		?>
			<tr>
				<td class="sq-backend-data">
					<?php text_box($prefix.'_urls['.$i.'][url]',  $url_info['url'], 40, 0, 'onChange="javascript: site_url_url_change('.$i.');"'); ?>
				</td>
		<?php
			for($k = 0; $k < count($this->protos); $k++) {
			?>
				<td class="sq-backend-data">
					<?php check_box($prefix.'_urls['.$i.'][protocols]['.$k.']', $this->protos[$k], !empty($url_info[$this->protos[$k]]), 'site_url_protocol_click('.$i.');'); ?>
				</td>
				<td class="sq-backend-data">
					<?php echo strtoupper($this->protos[$k]); ?>
				</td>
			<?php
			}// end for
		?>
			</tr>
		<?php
	}// _paintURL


	/**
	* Processes the URLs posted from the paintURLs fn above
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processURLs(&$asset, $prefix) 
	{

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
			}// end if
		}// end for

		$asset->saveWebURLs($urls);

	}// end processURLS()


	/**
	* Paint the interface for setting the design for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintDesign(&$asset, &$o, $prefix) 
	{

		$design_link = $asset->getLink(SQ_LINK_USES, 'design', false);
		$designid = (is_null($design_link)) ? 0 : $design_link['minorid'];

		$designs = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design', false, true);
		$contents = Array();
		foreach($designs as $id => $type_code) {
			$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($id, $type_code, true);
			if (is_null($design)) continue;
			$contents[$design->id] = $design->name.' :: '.$design->id;
		}

		asort($contents);
		combo_box($prefix.'_designid', $contents, false, $designid);

	}// end paintDesign

	/**
	* Processes the setting of the Design for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDesign(&$asset, $prefix) 
	{
		$design_link = $asset->getLink(SQ_LINK_USES, 'design', false);
		$designid = (is_null($design_link)) ? 0 : $design_link['minorid'];

		$new_designid = $_POST[$prefix.'_designid'];

		// if they are the same then there is nothing to do.
		if ($designid == $new_designid) return true;

		// if there is an exist entry delete it
		if ($designid) {
			if (!$asset->deleteLink($design_link['linkid'])) return false;
		}

		$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);
		if (is_null($design)) return false;

		if ($asset->createLink($design, SQ_LINK_USES)) {
			$asset->updateLookups();
			return true;
		} else {
			return false;
		}

	}// end processDesign()


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
		$pages = $asset->getLinks(SQ_LINK_NAV, 'page', false);

		if (!count($pages)) {
			echo '<i>There are no Pages that are children of this site.</i>';
			hidden_field($prefix.'_'.$page_type, '0');
			return;
		}
		
		$selected = 0;
		$contents = Array();
		foreach($pages as $info) {
			$page = &$GLOBALS['SQ_SYSTEM']->am->getAsset($info['minorid'], $info['type_code'], true);
			if (is_null($page)) continue;
			if ($info['value'] == $page_type) $selected = $page->id;
			$contents[$page->id] = $page->name;
		}

		combo_box($prefix.'_'.$page_type, $contents, false, $selected);

	}// end paintPageSelection()

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
		trigger_error('Processing of '.$page_type.' page not done', E_USER_NOTICE);
	}

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
	}// end paintIndexPage

	/**
	* Processes the setting of the index page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processIndexPage(&$asset, $prefix) 
	{
		$this->processPageSelection('index', $asset, $prefix);
	}

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
	}// end paintForbiddenPage

	/**
	* Processes the setting of the forbidden page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processForbiddenPage(&$asset, $prefix) 
	{
		$this->processPageSelection('forbidden', $asset, $prefix);
	}

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
		$this->paintPageSelection('not found', $asset, $prefix);
	}// end paintNotFoundPage

	/**
	* Processes the setting of the not found page for this site
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNotFoundPage(&$asset, $prefix) 
	{
		$this->processPageSelection('not found', $asset, $prefix);
	}

}//end class
?>
