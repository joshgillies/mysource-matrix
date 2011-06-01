<?php
/**
 * XML Lib.
 *
 * @version    4.0.0
 * @package    MySource4
 * @subpackage Libs
 * @author     Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license    http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */

/**
 * Library for handling data in XML form.
 *
 * @since 4.0.0
 */
class XML
{


    /**
     * Converts a DomNodeList to an array containing its' elements.
     *
     * @param DomNodeList $nodeList List of nodes to convert.
     *
     * @since  4.0.0
     * @return array
     */
    public static function nodeListToArray(DomNodeList $nodeList)
    {
        $retval = array();
        foreach ($nodeList as $item) {
            $retval[] = $item;
        }

        return $retval;

    }//end nodeListToArray()


    /**
     * Searches under the passed element for elements of any of the passed type.
     *
     * @param array   $tagNames An array of tag names to search for.
     * @param DomNode $elem     The element to search under.
     *
     * @since  4.0.0
     * @return array
     */
    public static function getElementsByTagNames(array $tagNames, DomNode $elem)
    {
        $retval = array();

        foreach ($tagNames as $tagName) {
            $elements = $elem->getElementsByTagName($tagName);
            foreach ($elements as $element) {
                $retval[] = $element;
            }
        }

        return $retval;

    }//end getElementsByTagNames()


    /**
     * Searches for given attribute value under the given element.
     *
     * If recusrive set to false then only 1 level will be searched.
     *
     * @param DomDocument $doc       Xml document.
     * @param DomNode     $element   Parent element.
     * @param string      $attr      Name of the attribute.
     * @param string      $value     Attribute value.
     * @param boolean     $recursive Recurse the dom tree.
     *
     * @since  4.0.0
     * @return DOMNodeList
     */
    public static function getElementsByAttributeValue(DomDocument $doc, DomNode $element, $attr, $value, $recursive=TRUE)
    {
        $nodes = array();
        if ($attr === '') {
            return $nodes;
        }

        $xp    = new DOMXPath($doc);
        $query = ($recursive === FALSE) ? '*' : '//*';
        $query = $query.'[@'.$attr.' = "'.$value.'"]';
        $nodes = $xp->query($query, $element);

        return $nodes;

    }//end getElementsByAttributeValue()


    /**
     * Returns TRUE if the given node has element node.
     *
     * Unlike hasChildNodes, hasChildElements returns false if it cannot find a
     * XML_ELEMENT_NODE.
     *
     * @param DomNode $node Dom node.
     *
     * @since  4.0.0
     * @return boolean
     */
    public static function hasChildElements(DomNode $node)
    {
        if ($node->hasChildNodes() === TRUE) {
            foreach ($node->childNodes as $child) {
                if ($child->nodeType === XML_ELEMENT_NODE) {
                    return TRUE;
                }
            }
        }

        return FALSE;

    }//end hasChildElements()


    /**
     * Returns the first child element node of the given node.
     *
     * Unlike fisrChild, getFirstChildElement returns the first XML_ELEMENT_NODE.
     * Returns NULL if non found.
     *
     * @param DomNode $node Dom node.
     *
     * @since  4.0.0
     * @return object
     */
    public static function getFirstChildElement(DomNode $node)
    {
        foreach ($node->childNodes as $child) {
            if ($child->nodeType === XML_ELEMENT_NODE) {
                return $child;
            }
        }

        return NULL;

    }//end getFirstChildElement()


    /**
     * Returns the child element node of the given node at the given position.
     *
     * @param DomNode $node     Dom node.
     * @param integer $position Position.
     *
     * @since  4.0.0
     * @return object
     */
    public static function getChildAt(DomNode $node, $position)
    {
        $pos = 0;
        foreach ($node->childNodes as $child) {
            if ($child->nodeType === XML_ELEMENT_NODE) {
                if ($pos === $position) {
                    return $child;
                }

                $pos++;
            }
        }

        return NULL;

    }//end getChildAt()


    /**
     * Returns the number of child nodes for specified node.
     *
     * @param DomNode $node Dom node.
     *
     * @since  4.0.0
     * @return integer
     */
    public static function childCount(DomNode $node)
    {
        $count = 0;
        foreach ($node->childNodes as $child) {
            if ($child->nodeType === XML_ELEMENT_NODE) {
                $count++;
            }
        }

        return $count;

    }//end childCount()


    /**
     * Traverse DOM tree from given start node.
     *
     * Returns details about each element it comes across and these elements'
     * children.
     *
     * @param DomElement $element Node to start from.
     *
     * @since  4.0.0
     * @return array
     */
    public static function traverseDom(DomElement $element)
    {
        $nodeInfo            = array();
        $nodeInfo['tagName'] = $element->tagName;

        if ($element->hasAttributes() === TRUE) {
            foreach ($element->attributes as $attribute) {
                $nodeInfo['attrs'][$attribute->name] = $attribute->value;
            }
        }

        if ($element->hasChildNodes() === TRUE) {
            foreach ($element->childNodes as $childNode) {
                if ($childNode->nodeType === XML_TEXT_NODE) {
                    $nodeInfo['value'] = trim($childNode->nodeValue);
                } else if ($childNode->tagName !== NULL) {
                    $nodeInfo['children'][] = self::traverseDOM($childNode);
                }
            }
        }

        return $nodeInfo;

    }//end traverseDom()


    /**
     * Allows multiple attributes of a DomElement to be set simultaneously.
     *
     * @param DomElement $element    The element to set the attributes for.
     * @param array      $attributes An associate array of the form:
     *                                 array(
     *                                  $attrname => $attrValue,
     *                                  ...,
     *                                 ).
     *
     * @since  4.0.0
     * @return DomElement
     */
    public static function setAttributes(DomElement $element, array $attributes)
    {
        foreach ($attributes as $attrName => $attrValue) {
            $element->setAttribute($attrName, $attrValue);
        }

        return $element;

    }//end setAttributes()


}//end class


?>
