#pragma once

class OrderItem
{
  public:
    OrderItem();
    ~OrderItem() = default;

    static OrderItem getItem(int itemId);
  
  private
};