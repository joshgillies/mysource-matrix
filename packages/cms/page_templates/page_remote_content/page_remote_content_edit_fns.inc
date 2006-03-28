<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_remote_content_edit_fns.inc,v 1.9 2006/03/28 03:42:29 sdanis Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Remote_Content_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Remote_Content_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint URL white list interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintWhiteList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$urls = $asset->attr('url_white_list');

		$urls = (is_array($urls)) ? implode("\n", $urls) : '';

		if ($write_access) {
			text_area($prefix.'_url_white_list', $urls, 60, 10);
		} else {
			echo nl2br($urls);
		}
		return $write_access;

	}//end paintWhiteList()


	/**
	* Process URL white list interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWhiteList(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$urls = array_get_index($_REQUEST, $prefix.'_url_white_list');

		$matches	= Array();
		$new_urls	= Array();

		if (preg_match_all('/\S+/i', $urls, $matches, PREG_SET_ORDER)) {
			foreach ($matches as $match) {
				$url = array_pop($match);
				$new_urls[$url] = $url;
			}
		}

		$asset->setAttrValue('url_white_list', $new_urls);
		return TRUE;

	}//end processWhiteList()


	/**
	* Prints a table of Post Variables that have been customised.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPostVariableTable(&$asset, &$o, $prefix)
	{
		ob_start();
			?><table width="100%" class="sq-backend-table">
				<tr>
					<th>Name</th>
					<th>Value</th><?php
				if ($asset->writeAccess('attributes')) {
					?><th>Delete</th><?php
				}
				?></tr><?php
			foreach ($asset->attr('post_variables') as $index => $variable) {
				?><tr>
					<td><?php echo $variable['name']; ?></td>
					<td><?php echo $variable['value']; ?></td><?php
				if ($asset->writeAccess('attributes')) {
					?><td><?php check_box($prefix.'_delete['.$index.']'); ?></td><?php
				}
				?></tr><?php
			}
			?></table><?php
		echo ob_get_clean();

		return TRUE;

	}//end paintPostVariableTable()


	/**
	* Prints an input area so that the user can specify an custom Post Variable.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewPostVariableInput(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		ob_start();
			text_box($prefix.'_name', '');
			echo ' ';
			text_box($prefix.'_value', '', 50);
		echo ob_get_clean();

		return TRUE;

	}//end paintNewPostVariableInput()


	/**
	* Creates / Deletes any customised Post Variables as necessary.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processNewPostVariableInput(&$asset, &$o, $prefix)
	{
		$post_variables = $asset->attr('post_variables');

		$delete_variables = Array();
		if (isset($_POST[$prefix.'_delete'])) {
			$delete_variables = array_keys($_POST[$prefix.'_delete']);
		}

		// Delete any necessary variables
		foreach ($delete_variables as $index) {
			unset($post_variables[$index]);
		}

		$create_variables = Array();
		if (isset($_POST[$prefix.'_name']) && !empty($_POST[$prefix.'_name'])) {
			$create_variables[trim($_POST[$prefix.'_name'])] = $_POST[$prefix.'_value'];
		}

		// Create any new variables, overwriting any existing ones.
		foreach ($create_variables as $name => $value) {
			// Checks to see if this particular post variable has been specified already.
			if (strpos(print_r($post_variables, TRUE), '[name] => '.$name."\n") === FALSE) {
				// Yes, we use numerical index because post vars can have names like
				// 'post_variable[name]', which would cause the delete checkboxes to break.
				$post_variables[] = Array(
										'name'	=> $name,
										'value'	=> $value,
									);
			} else {
				trigger_localised_error('CMS0059', E_USER_NOTICE);
			}
		}
		return $asset->setAttrValue('post_variables', $post_variables);

	}//end processNewPostVariableInput()


	/**
	* Display the Tag replacements
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintReplaceTagList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$tag_replacements = $asset->attr('tag_replacmenets');
		if (!empty($tag_replacements) || $write_access) {
			?>
			<table width="100%" class="sq-backend-table">
			<tr><th><?php echo translate('cms_remote_content_tag'); ?></th><th><?php echo translate('cms_remote_content_start_tag'); ?></th><th><?php echo translate('cms_remote_content_end_tag'); ?></th><th><?php echo translate('cms_remote_content_remove_contents'); ?></th><?php echo (($write_access && !empty($tag_replacements)) ? '<th>'.translate('delete').'</th>' : ''); ?></tr>
			<?php

			foreach ($tag_replacements as $tag => $tag_info) {
				if ($write_access) {
					?>
					<tr>
						<td><?php text_box($prefix.'_tag_replacments[reps]['.$tag.'][tag]', $tag, 30); ?></td>
						<td><?php text_box($prefix.'_tag_replacments[reps]['.$tag.'][start_tag]', $tag_info['start_tag'], 30); ?></td>
						<td><?php text_box($prefix.'_tag_replacments[reps]['.$tag.'][end_tag]', $tag_info['end_tag'], 30); ?></td>
						<td><?php check_box($prefix.'_tag_replacments[reps]['.$tag.'][remove_content]', TRUE, ($tag_info['remove_content'] ? TRUE : FALSE)); ?></td>
						<td><?php check_box($prefix.'_tag_replacments[reps]['.$tag.'][delete]'); ?>
					</tr>
					<?php
				} else {
				?>
					<tr>
					<td><?php echo htmlspecialchars($tag); ?></td>
					<td><?php echo htmlspecialchars($tag_info['start_tag']); ?></td>
					<td><?php echo htmlspecialchars($tag_info['end_tag']); ?></td>
					<td><?php echo ($tag_info['remove_content'] ? translate('yes') : translate('no')); ?></td>
					</tr>
				<?php
				}
			}//end foreach
		} else if (!$write_access) {
			echo translate('cms_remote_content_no_tag_replacements');
		}

		if ($write_access) {
			?>
			<tr>
				<td><?php text_box($prefix.'_tag_replacments[reps][new][tag]', '', 30); ?></td>
				<td><?php text_box($prefix.'_tag_replacments[reps][new][start_tag]', '', 30); ?></td>
				<td><?php text_box($prefix.'_tag_replacments[reps][new][end_tag]', '', 30); ?></td>
				<td><?php check_box($prefix.'_tag_replacments[reps][new][remove_content]'); ?></td>
				<?php echo (!empty($tag_replacements) ? '<td>&nbsp;</td>' : ''); ?>
			</tr>
			<?php
		}
		?>
		</table>
		<?php

	}//end paintReplaceTagList()


	/**
	* Process the Tag replacements
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processReplaceTagList(&$asset, &$o, $prefix)
	{
		$reps = $_POST[$prefix.'_tag_replacments']['reps'];

		$replacements = Array();
		foreach ($reps as $rep_info) {
			if (isset($rep_info['delete']) || empty($rep_info['tag'])) {
				continue;
			}

			$remove_content = FALSE;
			if (isset($rep_info['remove_content'])) {
				$remove_content = TRUE;
			}

			$replacements[$rep_info['tag']] = Array(
												'start_tag'			=> $rep_info['start_tag'],
												'end_tag'			=> $rep_info['end_tag'],
												'remove_content'	=> $remove_content,
											  );
		}

		if (!$asset->setAttrValue('tag_replacmenets', $replacements)) {
			return FALSE;
		}

		return TRUE;

	}//end processReplaceTagList()


}//end class

?>