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
* $Id: page_site_map.inc,v 1.9 2004/03/26 15:36:29 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page.inc';

/**
* Page_Standard
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
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
	function Page_Site_Map($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => false);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	* @return boolean
	*/
	function printBody()
	{
		$root_asset = null;
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', false, 'root');
		if (!empty($root_link)) {
			$root_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($root_link['minorid'], $root_link['minor_type_code']);
		}

		if (is_null($root_asset)) {
			// firstly, get the sites that this page belongs to
			$site_info = $GLOBALS['SQ_SYSTEM']->am->getRootURL();
			if (empty($site_info)) return false;
			$siteid = $site_info['assetid'];
		} else {
			$siteid = $root_asset->id;
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_asset);
			unset($root_asset);
		}

		$levels = ($this->attr('levels') != 0) ? $this->attr('levels') : null;

		$page_data = $GLOBALS['SQ_SYSTEM']->am->getAssetTree($siteid, $levels, array_keys($this->attr('exclude_list')));

		$assetids = Array();

		foreach ($page_data as $assetid => $data) {
			$assetids[] = $assetid;
			$assetids = array_merge($assetids, array_keys($data));
		}
		global $SQ_SITE_MAP_ASSET_URLS;

		$SQ_SITE_MAP_ASSET_URLS = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($assetids);

		?>
		<script language="javascript" type="text/javascript" src="<?php echo sq_web_path('lib').'/js/edit.js' ?>"></script>

		<?php
		// figure out the columns for the site map
		$num_columns = ($this->attr('columns') > 0) ? $this->attr('columns') : 1;
		if (empty($page_data)) return;
		$num_pages = count(array_keys($page_data[$siteid]));

		$num_chunks = ceil($num_pages / $num_columns);
		// CHUNK chunk CHUNK chunk CHUNK it up!
		$chunks = array_chunk($page_data[$siteid], $num_chunks, true);
		unset($page_data[$siteid]);
		?>
		<table cellpadding="0" cellspacing="0" border="0">
			 <tr>

			<?php
			foreach ($chunks as $chunk) {
				?><td valign="top"><?php
				$sub_chunk = Array();
				$sub_chunk[$siteid] = $chunk;
				$sub_chunk = $sub_chunk + $page_data;
				$this->_recursePrintAssetList($sub_chunk, $siteid, $this->attr('top_level'), $this->attr('stalks'));
				unset($sub_chunk);
				?></td><?php
			}
			?>

			</tr>
		</table>
		<?php

	}//end printBody()


	/**
	* Recursively prints a tree of the passed in data
	*
	* @param &Array		$page_data		the data to print in a tree structure
	* @param integer	$parent			the parent that we are currently at
	* @param boolean	$top_level		if TRUE the stalks printed at the top level
	* @param boolean	$generating		if TRUE content will be generated for a content file
	* @param boolean	$stalks			if TRUE stalks are printed
	*
	* @access private
	* @return boolean
	*/
	function _recursePrintAssetList(&$page_data, $parent, $top_level=false, $stalks=true, $generating=false)
	{
		if (!$generating) {
			global $SQ_SITE_MAP_ASSET_URLS;
		}
		?>
			<table cellpadding="0" cellspacing="0" border="0">
		<?php
			$num_kids = count($page_data[$parent]);
			$i = 0;
			foreach ($page_data[$parent] as $data) {
				$end = ($i == $num_kids - 1);
				$image1 = (!$top_level || !$stalks) ? 'blank.gif' : 'tree/stalk.gif';
				$image2 = (!$top_level || !$stalks) ? 'blank.gif' : 'tree/branch.gif';

				$bg = ($end) ? '' : 'background="'.sq_web_path('lib').'/web/images/'.$image1.'"';

				$url = ($generating) ? '<'.'?php echo $SQ_SITE_MAP_ASSET_URLS[$data[\'assetid\']] ?'.'>' : $SQ_SITE_MAP_ASSET_URLS[$data['assetid']];
			?>
				<tr>
					<td <?php echo $bg; ?>>
					<img src="<?php echo sq_web_path('lib').'/web/images/'.$image2; ?>" width="20" height="20" border="0" alt="branch" /></td>
					<td >
						<?php if ($this->attr('icons')) { ?>
							<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']); ?>", "16", "16", "");</script>&nbsp;&nbsp;
						<?php } ?>
						<a href="<?php echo $url?>"><?php echo $data['name'];?></a>
					</td>
				</tr>
			<?php
				if (!empty($page_data[$data['assetid']])) {
				?>
					<tr>
						<td <?php echo $bg; ?>><img src="<?php echo sq_web_path('lib').'/web/images/blank.gif'; ?>" width="1" height="1" alt="blank" /></td>
						<td>
					<?php
						$this->_recursePrintAssetList($page_data, $data['assetid'], true, $stalks, $generating);
					?>
						</td>
					</tr>
				<?php
				}//end if
				$i++;
			}//end foreach
		?>
			</table>
		<?php

		return true;

	}//end _printAssetTypeList()


}//end class

?>