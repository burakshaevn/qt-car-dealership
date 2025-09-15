#include <gtest/gtest.h>
#include "../include/user.h"

class UserTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_products.push_back({ "CLE 200 Cabriolet", "blue" });
        test_products.push_back({ "E 220 d Limousine", "red" });

        test_user_info = UserInfo(
            1,
            "John Doe",
            "john@example.com",
            "securepassword",
            Role::Admin,
            test_products
            );
    }

    QList<Products::ProductKey> test_products;
    UserInfo test_user_info;
};

// TEST_F(UserTest, default_constructor_creates_empty_user) {
//     UserInfo empty_info;
//     User user(empty_info);

//     EXPECT_EQ(user.GetId(), 0);
//     EXPECT_TRUE(user.GetName().isEmpty());
//     EXPECT_TRUE(user.GetEmail().isEmpty());
//     EXPECT_EQ(user.GetRole(), Role::Unknown);
//     EXPECT_TRUE(user.GetProducts().isEmpty());
// }

// TEST_F(UserTest, parameterized_constructor_sets_values_correctly) {
//     User user(test_user_info);

//     EXPECT_EQ(user.GetId(), 1);
//     EXPECT_EQ(user.GetName(), "John Doe");
//     EXPECT_EQ(user.GetEmail(), "john@example.com");
//     EXPECT_EQ(user.GetRole(), Role::Admin);
//     ASSERT_EQ(user.GetProducts().size(), 2);
//     EXPECT_EQ(user.GetProducts()[0], "Product1|Red");
// }

// TEST_F(UserTest, set_get_id_works_correctly) {
//     User user(test_user_info);
//     user.SetId(42);
//     EXPECT_EQ(user.GetId(), 42);
// }

// TEST_F(UserTest, set_get_name_works_correctly) {
//     User user(test_user_info);
//     user.SetName("Alice Smith");
//     EXPECT_EQ(user.GetName(), "Alice Smith");
// }

// TEST_F(UserTest, set_get_email_works_correctly) {
//     User user(test_user_info);
//     user.SetEmail("alice@example.com");
//     EXPECT_EQ(user.GetEmail(), "alice@example.com");
// }

// TEST_F(UserTest, set_get_role_works_correctly) {
//     User user(test_user_info);
//     user.SetRole(Role::User);
//     EXPECT_EQ(user.GetRole(), Role::User);
// }

// TEST_F(UserTest, set_get_products_works_correctly) {
//     User user(test_user_info);
//     QList<Products::ProductKey> new_products = {{"Mercedes-AMG CLE 53 4MATIC+ Coupé", "yellow"}};
//     user.SetProducts(new_products);

//     auto products = user.GetProducts();
//     ASSERT_EQ(products.size(), 1);
//     EXPECT_EQ(products[0], "Mercedes-AMG CLE 53 4MATIC+ Coupé");
//     EXPECT_EQ(products[0], "yellow");
// }

// TEST_F(UserTest, products_list_is_copied_not_referenced) {
//     User user(test_user_info);
//     auto original_products = user.GetProducts();
//     original_products.push_back({"Extra", "Yellow"});

//     auto current_products = user.GetProducts();
//     EXPECT_EQ(current_products.size(), 2);  // Оригинальные 2 продукта
//     EXPECT_NE(current_products.size(), 3);  // "Extra" не добавился
// }
