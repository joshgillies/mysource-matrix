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
* $Id: content_type_structure_tree.inc,v 1.6 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

/**
* Content_Type_Structure_Tree
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Structure_Tree extends Content_Type
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
	* Draws the tree based on the assets structure attribute
	*
	* @param boolean	$show_trailers	Whether to show lines between non-adjacent links
	*
	* @return string
	* @access public
	*/
	function drawTree($show_trailers=FALSE)
	{
		$tree_string = '';
		$classes = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_STRUCTURE_TREE_NODE_CLASSES');

		$tree_data = $this->attr('structure');
		$current_trailers = Array();
		ob_start();
		for ($treecount=0; $treecount < count($tree_data); $treecount++) {
			$data = $tree_data[$treecount];
			$indent = 0;
				for ($i = 0; $i < $data['indent']; $i++) {
					if ($i == ($data['indent'] - 1)) {
						// Check out whats after to see what to output
						if (($treecount == count($tree_data) - 1) || ($tree_data[$treecount + 1]['indent'] != $data['indent'])) {
							?>
							<span class="<?php echo $classes['node_branches']; ?>" style="margin-left: 10px;">|_</span>
							<?php
						} else {
							?>
							<span class="<?php echo $classes['node_branches']; ?>" style="margin-left: 10px;">|_</span>
							<?php
						}
					} else {
						$span_text = '&nbsp;';
						$class_text = '';
						if ($show_trailers && isset($current_trailers[$i])) {
							$span_text = '|';
							$class_text = 'class="'.$classes['node_branches'].'" ';
						}
						?>
						<span <?php echo $class_text; ?>style="margin-left: 10px;"><?php echo $span_text; ?></span>
						<?php
					}
				}

				// Get the styles for the content text
				$style_text = '';
				if ($data['bold'] != '0') {
					if ($style_text == '') $style_text = ' style="';
					$style_text .= 'font-weight: bold;';
				}
				if ($data['italic'] != '0') {
					if ($style_text == '') $style_text = ' style="';
					$style_text .= 'font-style: italic;';
				}
				if ($data['underline'] != '0') {
					if ($style_text == '') $style_text = ' style="';
					$style_text .= 'text-decoration: underline;';
				}
				if ($style_text != '') $style_text .= '"';
				?>
				<span class="<?php echo $classes['node_text']; ?>" <?php echo $style_text; ?>><?php echo $data['content']; ?></span><br />
				<?php
				// Below is a mess, but it unsets any trailers, then searches for new ones based on this
				if ($show_trailers) {
					unset($current_trailers[$data['indent'] - 1]);
					// Now we check to see if we should add some trailers, so this is linked to later instances
					if (isset($tree_data[$treecount + 1]) && ($tree_data[$treecount + 1]['indent'] > $data['indent'])) {
						for ($ja = $treecount + 2; $ja < count($tree_data); $ja++) {
							$next_data = $tree_data[$ja];
							if ($next_data['indent'] < $data['indent']) break;
							if ($next_data['indent'] == $data['indent']) {
								$current_trailers[$data['indent'] - 1] = 'true';
							}
						}
					}
				}
		}//end for
			$tree_string = ob_get_contents();
		ob_end_clean();

		return $tree_string;

	}//end drawTree()


}//end class

?>