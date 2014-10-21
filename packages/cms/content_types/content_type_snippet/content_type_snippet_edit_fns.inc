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
* $Id: content_type_snippet_edit_fns.inc,v 1.7 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Raw_HTML_Edit_Fns
*
* Purpose
*    Paint the interface for selecting a preconfigured library of snippets
*    using a dropdown box available in the snippet content type
*
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Snippet_Edit_Fns extends Content_Type_Edit_Fns
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
	* Prints the snippet content if it is being viewed from an edit interface,
	*  otherwise just calls printBody on the snippet asset
	*
	* @param object		&$asset		the nest content content type object
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we genenerating the cache file
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset, $editing=FALSE, $generating=FALSE)
	{
		$snippet_asset = $asset->getCurrentSnippet();
		if ($generating) {
			$this->paintGenerated($asset, $snippet_asset);
		} else {
			if (!is_null($snippet_asset)) {
				$snippet_asset->printBody();
			}
		}

		return TRUE;

	}//end paint()


	/**
	* Paints the content type as it should be cached
	*
	* @param object	&$asset			the table whose interface we are painting
	* @param object	&$snippet_asset	the snippet content type asset
	*
	* @return void
	* @access public
	*/
	function paintGenerated(&$asset, &$snippet_asset)
	{
		// this is used when generating content_file.php
		echo '<'.'?php'."\n";
		if (!is_null($snippet_asset)) {
			?>
			$snippet_assetid = '<?php echo $snippet_asset->id; ?>';
			$snippet_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($snippet_assetid);
			$snippet_asset->printBody();
			<?php
		}
		echo '?'.'>'."\n";

	}//end paintGenerated()


	/**
	* Prints the backend interface for the snippet selector
	*
	* @param object	&$asset	the snippet content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {

			// no access, do not change anything
			$snippets = $asset->getSnippets();
			if (empty($snippets)) {
				echo translate('You do not have access to any of the available snippets');

				return TRUE;
			}

			// get default values
			$source_safe = FALSE;
			$current_view = $asset->attr('view_style');
			$current_snippet = $asset->getCurrentSnippet();
			$current_id = (is_null($current_snippet)) ? 0 : $current_snippet->id;
			$current_content = translate('Please select a snippet from the dropdown box');

			if ($current_id) {
				if (!isset($snippets[$current_id])) {
					// no read access or snippet deleted
					$current_content = sprintf(translate('Warning: Unable to preview selected snippet: %s<br />Make sure you have effective read access to the selected snippet<br />Make sure the snippet is correctly listed under the snippet root'), get_asset_tag_line($current_id));

				} else {
					$current_content = $snippets[$current_id]['content'];
					$source_safe = TRUE;
				}
			}

			// prepare the snippet and view style selection
			$viewstyle = Array(
							'source'	=> 'View source',
							'html'		=> 'View HTML',
						 );
			$dropdown = Array(0 => '--');
			foreach ($snippets as $id => $info) {
				$dropdown[$id] = $id.': '.$info['name'];
				// print the content for quick switching later
				hidden_field($prefix.'_hidden_source_'.$id, $info['type'] == 'content_type_nest_content' ? $info['content'] : nl2br(htmlspecialchars($info['content'])));
				hidden_field($prefix.'_hidden_html_'.$id, $info['content']);
			}

			// print the interface
			// and the javascript for instant preview
			$switchSnippet = 'onChange="switchSnippet(this.value)"';
			$switchView = 'onChange="switchView(this.value)"';
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-cell-alt">
					<?php echo combo_box($prefix.'_dropdown', $dropdown, FALSE, $current_id, 0, $switchSnippet); ?>
					<?php echo combo_box($prefix.'_viewstyle', $viewstyle, FALSE, $current_view, 0, $switchView); ?>
					</td>
				</tr>
				<tr>
					<td>
						<span id="<?php echo $prefix.'_displayarea'; ?>" name="<?php echo $prefix.'_displayarea'; ?>">
						<?php
						if ($current_view == 'source' && $source_safe) {
							echo $snippets[$current_id]['type'] == 'content_type_nest_content' ? $current_content : nl2br(htmlspecialchars($current_content));
						} else {
							echo $current_content;
						}
						?>
						</span>
					</td>
				</tr>
			</table>
			<script type="text/javascript">
				function switchSnippet(id) {
					var span = document.getElementById('<?php echo $prefix.'_displayarea'; ?>');
					var style = document.getElementById('<?php echo $prefix.'_viewstyle'; ?>');
					if (id == 0) {
						span.innerHTML = '<?php echo translate('Please select a snippet from the dropdown box'); ?>';

						return;
					}
					var snippet = document.getElementById(('<?php echo $prefix; ?>_hidden_' + style.value + '_' + id));
					span.innerHTML = snippet.value;
				}
				function switchView(style) {
					var span = document.getElementById('<?php echo $prefix.'_displayarea'; ?>');
					var dropdown = document.getElementById('<?php echo $prefix.'_dropdown'; ?>');
					if (dropdown.value == 0) {
						return;
					}
					var snippet = document.getElementById(('<?php echo $prefix; ?>_hidden_' + style + '_' + dropdown.value));
					span.innerHTML = snippet.value;
				}
			</script>
			<?php
		}//end if

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface for this Raw HTML editor
	*
	* @param array	$link	information used to create the initial link
	* @param object	&$asset	the snippet content type
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {

			// update view style and selected snippet
			if (isset($_POST[$prefix.'_viewstyle'])) {
				$asset->setAttrValue('view_style', $_POST[$prefix.'_viewstyle']);
				$asset->saveAttributes();
			}
			$current_snippet = $asset->getCurrentSnippet();
			$current_id = (is_null($current_snippet)) ? 0 : $current_snippet->id;
			$new_id = (isset($_POST[$prefix.'_dropdown'])) ? $_POST[$prefix.'_dropdown'] : 0;

			if (!$new_id && (!$current_id || !$current_snippet->readAccess())) {
				// no new id set, and does not have access to current snippet
				// prevent accidental overwrite
				return FALSE;
			}

			if ($new_id) {
				// user selected a different snippet
				if ($new_id != $current_id) {
					if ($current_id) {
						// delete old snippet link
						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'snippet_asset');
						if (!$asset->deleteLink($link['linkid'])) {
							return FALSE;
						}
					}
					$new_snippet = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_id);
					// create new snippet link
					if (!$asset->createLink($new_snippet, SQ_LINK_NOTICE, 'snippet_asset')) {
						return FALSE;
					}
					// special initial case, current = 0
					if (!$current_id && $new_id) {
						$asset->linksUpdated();
					}
				}//end if

			} else if ($current_id) {
				// user unselected a snippet
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', TRUE, 'snippet_asset');
				if (!$asset->deleteLink($link['linkid'])) {
					return FALSE;
				}
			}

			// update content.php file
			$bc_container_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE, 'minor');
			if (!empty($bc_container_link)) {
				$container = $GLOBALS['SQ_SYSTEM']->am->getAsset($bc_container_link[0]['majorid']);
				$edit_fns = $container->getEditFns();
				$edit_fns->generateContentFile($container);
			}

		}//end if
		return FALSE;

	}//end processBackend()


}//end class

?>