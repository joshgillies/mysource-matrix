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
* $Id: page_site_map.inc,v 1.44 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Page_Standard
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.44 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Site_Map extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


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
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		switch (strtolower($link['value'])) {
			case 'root' :
				return translate('cms_root_node_link_desc', translate('cms_site_map_asset_name'));;
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @return boolean
	* @access public
	*/
	function printBody()
	{
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$rootid = $this->_getRootNodeId();
		$cache_key = $rootid.'-ctx'.$contextid;

		// Try to load from cache, with our root node as cache key
		$cached_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);

		// if the cache is empty, go ahead and regenerate a new version
		if ($cached_contents === FALSE) {

			$levels = ($this->attr('levels') != 0) ? $this->attr('levels') : NULL;

			$page_data = $GLOBALS['SQ_SYSTEM']->am->getAssetTree($rootid, $levels, array_keys($this->attr('exclude_list')), $this->attr('show_type_2') ? SQ_SC_LINK_BACKEND_NAV : SQ_SC_LINK_FRONTEND_NAV, $this->attr('include_types'), $this->attr('include_dependants'));

			$assetids = Array();

			foreach ($page_data as $assetid => $data) {
				$assetids[] = $assetid;
				$assetids = array_merge($assetids, array_keys($data));
			}
			global $SQ_SITE_MAP_ASSET_URLS;

			$SQ_SITE_MAP_ASSET_URLS = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($assetids);

			ob_start();
				if ($this->attr('icons')) {
					echo '<script type="text/javascript" src="'.sq_web_path('lib').'/js/general.js"></script>';
				}
				?>

				<?php
				// figure out the columns for the site map
				$num_columns = ($this->attr('columns') > 0) ? $this->attr('columns') : 1;
				if (empty($page_data)) {
					ob_end_clean();
					return;
				}

				$num_pages = count(array_keys($page_data[$rootid]));

				$num_chunks = ceil($num_pages / $num_columns);
				// CHUNK chunk CHUNK chunk CHUNK it up!
				$chunks = array_chunk($page_data[$rootid], $num_chunks, TRUE);
				$root_data = $page_data[$rootid];
				unset($page_data[$rootid]);

				// better than conditional html everywhere, well have different functions
				// for each format
				switch ($this->attr('map_format')) {
					case 'unordered_list':
						$this->_paintUnorderedList($root_data, $page_data);
					break;

					default:
						$this->_paintDefault($chunks, $page_data, $rootid);
				}

				$cached_contents = ob_get_contents();
				$cm->saveToCache($this->id, $this->type(), $cache_key, $cached_contents);
			ob_end_clean();

		}//end if cached

		echo $cached_contents;

	}//end printBody()


	/**
	* Get the ID of the effective root node for this page
	*
	* @return int
	* @access private
	*/
	function _getRootNodeId()
	{
		$root_asset = NULL;
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'asset', FALSE, 'root');
		if (!empty($root_link)) {
			$root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid'], $root_link['minor_type_code']);
		}

		if (is_null($root_asset)) {
			// firstly, get the sites that this page belongs to
			$site_info = $GLOBALS['SQ_SYSTEM']->am->getRootURL();
			if (empty($site_info)) return FALSE;
			$rootid = $site_info['assetid'];
		} else {
			$rootid = $root_asset->id;

			$parameter_map = $this->getAttribute('parameter_map');
			$dynamic_root = $parameter_map->getParameterValue('root_node');
			if (!empty($dynamic_root)) {
				$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($dynamic_root);
				if (isset($parents[$rootid])) {
					// dynamic root is a child of the static root, so can be used
					$rootid = $dynamic_root;
				}
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
			unset($root_asset);
		}
		return $rootid;

	}//end _getRootNodeId()


	/**
	* Begins to paint a site map in the default format
	*
	* @param array		$chunks		the level of depth with which to begin processing
	* @param array		&$page_data	data on assets
	* @param int		$level		Level
	* @param boolean	$generating	Generating
	*
	* @return void
	* @access private
	*/
	function _paintUnorderedList($chunks, &$page_data, $level=1, $generating=FALSE)
	{
		$top_level = $this->attr('top_level');
		if ($top_level == 0) $level = -1;

		if (!$generating) global $SQ_SITE_MAP_ASSET_URLS;

		$display_formats = $this->attr('display_formats');

		?>
		<ul>
<?php
		foreach ($chunks as $item) {
			$url = ($generating) ? '<'.'?php echo $SQ_SITE_MAP_ASSET_URLS[$item[\'assetid\']] ?'.'>' : $SQ_SITE_MAP_ASSET_URLS[$item['assetid']];
?>
			<li><?php
				if ($this->attr('icons')) {
					sq_print_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($item['type_code']), '16', '16');
					echo '&nbsp;&nbsp;';
				}

				$format = $this->_getDisplayFormat($item);
			?><a href="<?php echo $url;?>" title="<?php echo $item['name']; ?>"><?php echo $format; ?></a>
<?php
			if (isset($page_data[$item['assetid']]) && $level + 1 <= $top_level) {
?>
			<?php
				$this->_paintUnorderedList($page_data[$item['assetid']], $page_data, $level + 1, $generating);
			}
?>
			</li>
			<?php

		}
		?>
		</ul>
<?php

	}//end _paintUnorderedList()


	/**
	* Begins to paint a site map in the default format
	*
	* @param array	$chunks		chunks of assets to buuild the map with
	* @param array	$page_data	data on assets
	* @param int	$rootid		id of root asset
	*
	* @return void
	* @access private
	*/
	function _paintDefault($chunks=Array(), $page_data, $rootid)
	{
		?>

		<table cellpadding="0" cellspacing="0" border="0">
		<tr>
		<?php
		foreach ($chunks as $chunk) {
			?><td valign="top"><?php
			$sub_chunk = Array();
			$sub_chunk[$rootid] = $chunk;
			$sub_chunk = $sub_chunk + $page_data;
			$this->_defaultRecursePrintAssetList($sub_chunk, $rootid, 1);
			unset($sub_chunk);
			?></td><?php
		}
		?>
		</tr>
		</table>
		<?php

	}//end _paintDefault()


	/**
	* Recursively prints a tree of the passed in data
	*
	* @param array		&$page_data	the data to print in a tree structure
	* @param int		$parent		the parent that we are currently at
	* @param int		$level		the level of the tree we are printing
	* @param boolean	$generating	if TRUE content will be generated for a content file
	*
	* @return boolean
	* @access private
	*/
	function _defaultRecursePrintAssetList(&$page_data, $parent, $level, $generating=FALSE)
	{
		$top_level = $this->attr('top_level');
		$stalks = $this->attr('stalks');

		if (!$generating) global $SQ_SITE_MAP_ASSET_URLS;

		?>
		<table cellpadding="0" cellspacing="0" border="0">
			<?php
			$num_kids = count($page_data[$parent]);
			$i = 0;
			foreach ($page_data[$parent] as $data) {
				$end = ($i == $num_kids - 1);
				$image1 = (($level == 1 && !$top_level) || !$stalks) ? 'blank.gif' : 'tree/stalk.gif';
				$image2 = (($level == 1 && !$top_level) || !$stalks) ? 'blank.gif' : 'tree/branch.gif';

				$bg = ($end) ? '' : 'style="background-image: url(\''.sq_web_path('lib').'/web/images/'.$image1.'\')"';

				$url = ($generating) ? '<'.'?php echo $SQ_SITE_MAP_ASSET_URLS[$data[\'assetid\']] ?'.'>' : $SQ_SITE_MAP_ASSET_URLS[$data['assetid']];
				?>
				<tr>
					<td <?php echo $bg; ?>>
					<img src="<?php echo sq_web_path('lib').'/web/images/'.$image2; ?>" width="20" height="20" border="0" alt="Level <?php echo $level; ?> Item" /></td>
					<td >
						<?php
						if ($this->attr('icons')) {
						    sq_print_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']), '16', '16');
						    echo '&nbsp;&nbsp;';
						}
						$format = $this->_getDisplayFormat($data);
						?><a href="<?php echo $url?>" title="<?php echo $data['name']; ?>"><?php echo $format; ?></a>
					</td>
				</tr>
				<?php
				if (!empty($page_data[$data['assetid']])) {
					?>
					<tr>
						<td <?php echo $bg; ?>><img src="<?php echo sq_web_path('lib').'/web/images/blank.gif'; ?>" width="1" height="1" alt=" " /></td>
						<td>
							<?php
							$this->_defaultRecursePrintAssetList($page_data, $data['assetid'], ($level +1), $generating);
							?>
						</td>
					</tr>
					<?php
				}
				$i++;
			}//end foreach
			?>
		</table>
		<?php

		return TRUE;

	}//end _defaultRecursePrintAssetList()


	/**
	* Returns the display text for a site map item
	*
	* @param array	$data	Information about the site map item
	*
	* @return boolean
	* @access private
	*/
	function _getDisplayFormat($data)
	{
		if (!is_array($data)) return '';

		$display_formats = $this->attr('display_formats');

		$format = $data['name'];
		$format_type_code = NULL;
		if (isset($display_formats[$data['type_code']])) {
			$format_type_code = $data['type_code'];
		} else if (!empty($display_formats)) {
			foreach ($display_formats as $type => $info) {
				if ($info['inherit'] == 1) {
					if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($data['type_code'], $type)) {
						$format_type_code = $type;
						break;
					}
				}
			}
		}

		if (!is_null($format_type_code)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($data['assetid']);
			$format = $display_formats[$format_type_code]['format'];

			preg_match_all('/%([^%]*)%/i', $format, $matches);
			foreach ($matches[1] as $key => $keyword) {
				$rep = $asset->getKeywordReplacement($keyword);
				$format = preg_replace('!' . preg_quote($matches[0][$key], '!') . '!', $rep, $format);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		}

		return $format;

	}//end _getDisplayFormat()


}//end class

?>
