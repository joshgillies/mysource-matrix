<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_xml_edit_fns.inc,v 1.10 2012/08/30 00:58:16 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';

/**
* Data_Source_Xml_Edit_Fns
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Xml_Edit_Fns extends Data_Source_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paints the interface for entering XML feed details
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintFeed(&$asset, &$o, $prefix)
	{
		$url = html_entity_decode($asset->attr('url'));
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_box($prefix.'feed_url', $url, 100);
			return TRUE;
		} else {
			echo $url;
		}
		return FALSE;

	}//end paintFeed()


	/**
	* Processes the interface for entering XML feed details
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processFeed(&$asset, &$o, $prefix)
	{
		$url = htmlentities($asset->attr('url'));
		if (isset($_POST[$prefix.'feed_url'])) {
			$link = htmlentities(trim($_POST[$prefix.'feed_url']));
			if (strcasecmp($url, $link) != 0) {
				$asset->setAttrValue('url', $link);
				return TRUE;
			}
		}
		return FALSE;

	}//end processFeed()


	/**
	* Paints the interface for entering comma-separated tag names
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintTags(&$asset, &$o, $prefix)
	{
		$tags = unserialize(html_entity_decode($asset->attr('tags')));
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_area($prefix.'tag_names', !empty($tags) ? implode(',', $tags) : '', 60);
			return TRUE;
		} else {
			echo !empty($tags) ? implode(',', $tags) : '';
		}
		return FALSE;

	}//end paintTags()


	/**
	* Processes the interface for entering comma-separated tag names
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processTags(&$asset, &$o, $prefix)
	{
		$current_tags = htmlentities($asset->attr('tags'));
		if (isset($_POST[$prefix.'tag_names'])) {
			$posted_tags = htmlentities($_POST[$prefix.'tag_names']);
			if (strcasecmp($current_tags, $posted_tags) != 0) {
				$exploded = explode(',', $posted_tags);
				// trim excess whitespace for accuracy
				array_walk($exploded, Array(&$this, '_trimValue'));
				$asset->setAttrValue('tags', serialize($exploded));
				return TRUE;
			}
		}
		return FALSE;

	}//end processTags()


	/**
	* Paints the interface for entering comma-separated tag attribute names
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintTagAttributes(&$asset, &$o, $prefix)
	{
		$tags = unserialize(html_entity_decode($asset->attr('tags')));
		$tag_attributes = unserialize(html_entity_decode($asset->attr('tag_attributes')));
		$wa = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Tag');?></th>

				<th><?php echo translate('Attributes');?></th>

			</tr>
			<?php
			for ($i=0; $i<count($tags); $i++) {
				?>
				<tr>
					<td><?php echo $tags[$i];?> </td>
					<?php if ($wa) { ?>
						<td><?php text_box($prefix.'tagattr_'.$i, isset($tag_attributes[$tags[$i]]) ? implode(',', $tag_attributes[$tags[$i]]) : '', 30); ?> </td>
					<?php } else { ?>
						<td><?php echo isset($tag_attributes[$tags[$i]]) ? implode(',', $tag_attributes[$tags[$i]]) : ''; ?> </td>
					<?php } ?>
				</tr>
			<?php
			}
		?>
		</table>
		<?php

		return $wa;

	}//end paintTagAttributes()


	/**
	* Processes the interface for entering comma-separated tag attribute names
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processTagAttributes(&$asset, &$o, $prefix)
	{
		$i = 0;
		$tags = unserialize(html_entity_decode($asset->attr('tags')));
		$results = Array();
		foreach ($_POST as $tag => $attributes) {
			if ($tag === $prefix.'tagattr_'.$i) {
					$exploded = explode(',', htmlentities($attributes));
					// trim excess whitespace for accuracy
					array_walk($exploded, Array(&$this, '_trimValue'));
					if (isset($tags[$i])) $results[$tags[$i]] = $exploded;
				$i++;
			}
		}
		if (!empty($results)) {
			$asset->setAttrValue('tag_attributes', serialize($results));
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processTagAttributes()


	/**
	* Trims string by reference
	*
	* @param string	&$value	value to trim
	*
	* @return void
	* @access private
	*/
	function _trimValue(&$value)
	{
		$value = trim($value);

	}//end _trimValue()


	/**
	* Paints the interface for entering comma-separated tag names to be enlosed in CDATA
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintCDATATags(&$asset, &$o, $prefix)
	{
		$tags = unserialize(html_entity_decode($asset->attr('cdata_tags')));
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_area($prefix.'cdata_tag_names', !empty($tags) ? implode(',', $tags) : '', 60);
			return TRUE;
		} else {
			echo !empty($tags) ? implode(',', $tags) : '';
		}
		return FALSE;

	}//end paintTags()


	/**
	* Process the interface for entering comma-separated tag names to be enlosed in CDATA
	*
	* @param object	&$asset	the Data_Source_Xml asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processCDATATags(&$asset, &$o, $prefix)
	{
		$current_tags = htmlentities($asset->attr('cdata_tags'));
		if (isset($_POST[$prefix.'cdata_tag_names'])) {
			$posted_tags = htmlentities($_POST[$prefix.'cdata_tag_names']);

			if (strcasecmp($current_tags, $posted_tags) != 0) {
				$exploded = explode(',', $posted_tags);
				// trim excess whitespace for accuracy
				array_walk($exploded, Array(&$this, '_trimValue'));
				$asset->setAttrValue('cdata_tags', serialize($exploded));
				// overwrite the cache with this new XML feed url
				$asset->setResultSet(Array(), $asset->attr('url'));
				$asset->getResultSet($asset->attr('url'));
				return TRUE;
			}
		}
		return FALSE;

	}//end processTags()


}//end class

?>
