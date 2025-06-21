read -p "Enter a fruit: " fruit
case "$fruit" in
    apple)
        echo "Apple pie coming up!" ;;
    banana|plantain)
        echo "Banana smoothie!" ;;
    *)
        echo "I don't know that fruit." ;;
esac
