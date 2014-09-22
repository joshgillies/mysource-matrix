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
* $Id: page_remote_content_edit_fns.inc,v 1.17 2012/09/17 23:01:42 akarelia Exp $
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
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for controlling URL rewriting restrictions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRewritingRestrictions(&$asset, &$o, $prefix)
	{
		$options = $asset->attr('url_rewriting_options');
		$overall = 'none';
		foreach ($options as $i => $v) {
			if ($v) {
				if ($i == 'all') {
					$overall = 'all';
				} else {
					$overall = 'some';
				}
				break;
			}
		}

		if ($asset->writeAccess('attributes')) {

			radio_button($prefix.'_rewrite_type', 'none', ($overall == 'none'), '', 'id="'.$prefix.'_rewrite_type_none"');
			label(translate('No URLs may be tunneled'), $prefix.'_rewrite_type_none');

			echo '<br />';

			radio_button($prefix.'_rewrite_type', 'all', ($overall == 'all'), '', 'id="'.$prefix.'_rewrite_type_all"');
			label(translate('Any URL may be tunneled'), $prefix.'_rewrite_type_all');

			echo '<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
			echo translate('<i><b>NOTE: </b>Enabling this option can result in System being used as a Proxy to access <b>any domain</b><i>.');

			echo '<br />';

			radio_button($prefix.'_rewrite_type', 'some', ($overall == 'some'), '', 'id="'.$prefix.'_rewrite_type_some"');
			label(translate('Only the following URLs may be tunneled:'), $prefix.'_rewrite_type_some');

			echo '<br />';

			$pf = $prefix.'_rewrite_some_options';
			?>

			<div style="margin-left: 5ex" id="<?php echo $prefix; ?>_rewrite_type_some_options">
				<input type="checkbox" name="<?php echo $pf; ?>[]" value="remote_url" id="<?php echo $pf; ?>_remote_url" <?php if ($options['remote_url']) echo 'checked="checked"'; ?> <?php if ($overall != 'some') echo 'disabled="disabled"'; ?> />
				<label for="<?php echo $pf; ?>_remote_url"><?php echo translate('URLs underneath the Remote URL above'); ?></label>

				<br />

				<input type="checkbox" name="<?php echo $pf; ?>[]" value="remote_domain" id="<?php echo $pf; ?>_remote_domain" <?php echo $options['remote_domain'] ? 'checked="checked"' : ''; ?> <?php if ($overall != 'some') echo 'disabled="disabled"'; ?> />
				<label for="<?php echo $pf; ?>_remote_domain"><?php echo translate('URLs in the same domain as the Remote URL above'); ?></label>

				<br />

				<input type="checkbox" name="<?php echo $pf; ?>[]" value="white_list" id="<?php echo $pf; ?>_white_list" <?php if ($options['white_list']) echo 'checked="checked"'; ?> <?php if ($overall != 'some') echo 'disabled="disabled"'; ?> />
				<label for="<?php echo $pf; ?>_white_list"><?php echo translate('URLs based the following \'white list\' URLs:'); ?></label>

				<br />
				<textarea name="<?php echo $prefix.'_white_list'; ?>" onfocus="document.getElementById('<?php echo $pf; ?>_white_list').checked = true" <?php if ($overall != 'some') echo 'disabled="disabled"'; ?>><?php
					$urls = $asset->attr('url_white_list');
					echo (is_array($urls)) ? implode("\n", $urls) : '';
				?></textarea>
				<br /><em>(Enter one URL per line)</em>
			</div>

			<script type="text/javascript"><!--//<![CDATA[
				var onclickfn = new Function("toggleOptionDisplay(this);");
				var inputs = document.getElementsByName('<?php echo $prefix; ?>_rewrite_type');
				for (var i=0; i < inputs.length; i++) {
					inputs[i].onclick = onclickfn;
				}
				function toggleOptionDisplay(checkbox)
				{
					var siblings = document.getElementsByName(checkbox.name);
					for (var j=0; j < siblings.length; j++) {
						var container = document.getElementById(siblings[j].id+'_options');
						if (container == null) continue;
						var tags = ['INPUT', 'SELECT', 'TEXTAREA'];
						for (t in tags) {
							var inputs = container.getElementsByTagName(tags[t]);
							for (var i=0; i < inputs.length; i++) {
								inputs[i].disabled = (siblings[j] != checkbox);
							}
						}
					}
				}
			//]]> -->
			</script>
			<?php
			return TRUE;

		} else {

			// read only
			echo translate('cms_remote_content_tunneling_rule_'.$overall);
			if ($overall == 'some') {
				echo '<ul style="margin-top: 3px">';
				foreach (Array('remote_url', 'remote_domain', 'white_list') as $type) {
					if ($options[$type]) {
						echo '<li>'.translate('cms_remote_content_tunneling_option_'.$type);
						if ($type == 'white_list') {
							echo '<br />'.implode('<br />', $asset->attr('url_white_list'));
						}
						echo '</li>';
					}
				}
			}

			return FALSE;
		}


	}//end paintRewritingRestrictions()


	/**
	* Process the interface for controlling URL rewriting restrictions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRewritingRestrictions(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess()) {
			$options = $asset->attr('url_rewriting_options');
			if (isset($_POST[$prefix.'_white_list'])) {
				$white_list = explode("\n", trim($_POST[$prefix.'_white_list']));
				// strip off protocols if necessary
				foreach ($white_list as $i => $v) {
					if (empty($v)) unset($white_list[$i]);
					if (FALSE !== ($x = strpos(substr($v, 0, 7), '://'))) {
						$white_list[$i] = substr($v, $x+3);
					}
				}
				$asset->setAttrValue('url_white_list', $white_list);
			}
			switch ($_POST[$prefix.'_rewrite_type']) {
				case 'all':
					foreach ($options as $i => $v) {
						$options[$i] = FALSE;
					}
					$options['all'] = TRUE;
				break;
				case 'none':
					foreach ($options as $i => $v) {
						$options[$i] = FALSE;
					}
				break;
				case 'some':
					$posted_options = array_get_index($_POST, $prefix.'_rewrite_some_options', Array());
					foreach ($options as $i => $v) {
						$options[$i] = in_array($i, $posted_options);
					}
					if (empty($white_list)) {
						$options['white_list'] = FALSE;
					}
				break;
			}
			$asset->setAttrValue('url_rewriting_options', $options);
			return TRUE;

		}//end if write access

		return FALSE;

	}//end processRewritingRestrictions()


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

		$tag_replacements = $asset->attr('tag_replacements');
		if (!empty($tag_replacements) || $write_access) {
			?>
			<table width="100%" class="sq-backend-table">
			<tr><th><?php echo translate('Tag'); ?></th><th><?php echo translate('Start Tag'); ?></th><th><?php echo translate('End Tag'); ?></th><th><?php echo translate('Remove Contents'); ?></th><?php echo (($write_access && !empty($tag_replacements)) ? '<th>'.translate('Delete').'</th>' : ''); ?></tr>





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
					<td><?php echo ($tag_info['remove_content'] ? translate('Yes') : translate('No')); ?></td>


					</tr>
				<?php
				}
			}//end foreach
		} else if (!$write_access) {
			echo translate('No tag replacements defined');

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

		if (!$asset->setAttrValue('tag_replacements', $replacements)) {
			return FALSE;
		}

		return TRUE;

	}//end processReplaceTagList()


}//end class

?>
